#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define EPSILON 0.15
#define SHADOW_OPACITY 0.5

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 fragPos;
layout (location = 3) in vec3 fragNormal;
layout (location = 4) in vec4 fragPosWorld;
layout (location = 5) in vec3 fragTangent;

layout (location = 0) out vec4 outNormal;
layout (location = 1) out vec4 outAlbedo;

struct GameObject {
    vec4 position;
    vec4 rotation; // color for point lights
    vec4 scale; // radius for point lights

    mat4 modelMatrix;
    mat4 normalMatrix;

    int hasTexture;
    int hasNormal;

    int hasHeight;
    float heightscale;
    float parallaxBias;
    float numLayers;
    int parallaxmode;

    int isPointLight;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 inverseProjection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    int numGameObjects;
} ubo;

layout(set = 0, binding = 1) readonly buffer GameObjects {
    GameObject objects[];
} ssbo;

layout(push_constant) uniform Push {
    int objectId;
} push;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];
//layout(set = 1, binding = 1) uniform samplerCube shadowCubeMap;
layout(set = 2, binding = 0) uniform sampler2D normalSampler[];
layout(set = 3, binding = 0) uniform sampler2D heightSampler[];

vec2 RayMarch(vec2 st0_in, vec2 st1_in)
{
    float lod_base = textureQueryLod(heightSampler[push.objectId], st0_in).y;
    vec2 dims = textureSize(heightSampler[push.objectId], 0);
    float distInPix = length(dims * (st1_in-st0_in));

    const int iterations = 3;
    vec3 st0 = vec3(st0_in, 0.0);
    vec3 st1 = vec3(st1_in, -1.0);

    float nrStepsAlongRay = ssbo.objects[push.objectId].numLayers;			// very brute-force
    float scale = ssbo.objects[push.objectId].heightscale;

    float nrInnerIts = (nrStepsAlongRay + 7) / 8;

    float t0 = 0.0, t1 = 1.0;
    for(int i = 0; i < iterations; i++)
    {
        bool notStopped = true;
        int j = 0;

        while(notStopped && j < nrInnerIts)
        {
            float T1 = mix(t0, t1, clamp((j*8+1)*scale, 0.0, 1.0) );
            float T2 = mix(t0, t1, clamp((j*8+2)*scale, 0.0, 1.0) );
            float T3 = mix(t0, t1, clamp((j*8+3)*scale, 0.0, 1.0) );
            float T4 = mix(t0, t1, clamp((j*8+4)*scale, 0.0, 1.0) );
            float T5 = mix(t0, t1, clamp((j*8+5)*scale, 0.0, 1.0) );
            float T6 = mix(t0, t1, clamp((j*8+6)*scale, 0.0, 1.0) );
            float T7 = mix(t0, t1, clamp((j*8+7)*scale, 0.0, 1.0) );
            float T8 = mix(t0, t1, clamp((j*8+8)*scale, 0.0, 1.0) );

            float h1 = textureLod(heightSampler[push.objectId], mix(st0, st1, T1).xy, lod_base).r - 1.0;
            float h2 = textureLod(heightSampler[push.objectId], mix(st0, st1, T2).xy, lod_base).r - 1.0;
            float h3 = textureLod(heightSampler[push.objectId], mix(st0, st1, T3).xy, lod_base).r - 1.0;
            float h4 = textureLod(heightSampler[push.objectId], mix(st0, st1, T4).xy, lod_base).r - 1.0;
            float h5 = textureLod(heightSampler[push.objectId], mix(st0, st1, T5).xy, lod_base).r - 1.0;
            float h6 = textureLod(heightSampler[push.objectId], mix(st0, st1, T6).xy, lod_base).r - 1.0;
            float h7 = textureLod(heightSampler[push.objectId], mix(st0, st1, T7).xy, lod_base).r - 1.0;
            float h8 = textureLod(heightSampler[push.objectId], mix(st0, st1, T8).xy, lod_base).r - 1.0;

            float t_s = t0, t_e = t1;

            if(notStopped) { if( mix(st0, st1, T1).z >= h1 ) { t_s = T1; } else { t_e = T1; notStopped=false; } }
            if(notStopped) { if( mix(st0, st1, T2).z >= h2 ) { t_s = T2; } else { t_e = T2; notStopped=false; } }
            if(notStopped) { if( mix(st0, st1, T3).z >= h3 ) { t_s = T3; } else { t_e = T3; notStopped=false; } }
            if(notStopped) { if( mix(st0, st1, T4).z >= h4 ) { t_s = T4; } else { t_e = T4; notStopped=false; } }
            if(notStopped) { if( mix(st0, st1, T5).z >= h5 ) { t_s = T5; } else { t_e = T5; notStopped=false; } }
            if(notStopped) { if( mix(st0, st1, T6).z >= h6 ) { t_s = T6; } else { t_e = T6; notStopped=false; } }
            if(notStopped) { if( mix(st0, st1, T7).z >= h7 ) { t_s = T7; } else { t_e = T7; notStopped=false; } }
            if(notStopped) { if( mix(st0, st1, T8).z >= h8 ) { t_s = T8; } else { t_e = T8; notStopped=false; } }

            t0 = t_s; t1 = t_e;
            notStopped = notStopped && t0 < t1;

            ++j;
        }

        // update number of taps along ray we allow
        nrStepsAlongRay = int(clamp(4 * distInPix * abs(t1 - t0), 8, 16));
        scale = 1.0 / float(nrStepsAlongRay);
        nrInnerIts = (nrStepsAlongRay + 7) / 8;

        ++i;
    }

    float h0 = textureLod(heightSampler[push.objectId], mix(st0, st1, t0).xy, lod_base).r - 1.0;
    float h1 = textureLod(heightSampler[push.objectId], mix(st0, st1, t1).xy, lod_base).r - 1.0;
    float ray_h0 = mix(st0, st1, t0).z;
    float ray_h1 = mix(st0, st1, t1).z;

    vec3 eqR = vec3(-(ray_h1 - ray_h0), t1 - t0, 0.0);
    eqR.z = -dot(eqR.xy, vec2(0.0, ray_h0));		// 0.0 corresponds to t0

    vec3 eqT = vec3(-(h1 - h0), t1 - t0, 0.0);
    eqT.z = -dot(eqT.xy, vec2(0.0, h0));					// 0.0 corresponds to t0

    const float eps = 1.192093e-15F;
    float determ = eqR.x * eqT.y - eqR.y * eqT.x;
    determ = (determ < 0.0 ? -1.0 : 1.0) * max(eps, abs(determ));

    //Ar*t + Br*h  = -Cr
    //At*t + Bt*h  = -Ct

    float finalT = clamp(t0 + ((eqT.y * (-eqR.z) + (-eqR.y) * (-eqT.z)) / determ), 0.0, 1.0);

    //float finalT = saturate(0.5*(t0+t1));
    return mix(st0_in, st1_in, finalT).xy - st0_in;
}

vec2 projectVecToTextureSpace(vec3 dir, vec2 texST, float bumpScale, bool skipProj, vec3 dPdx, vec3 dPdy, vec3 nrmBaseNormal)
{
    vec2 texDx = dFdx(texST);
    vec2 texDy = dFdy(texST);
    vec3 vR1 = cross(dPdy, nrmBaseNormal);
    vec3 vR2 = cross(nrmBaseNormal, dPdx);
    float det = dot(dPdx, vR1);
    const float eps = 1.192093e-15F;
    float sgn = det < 0.0 ? -1.0 : 1.0;
    float s = sgn / max(eps, abs(det));
    vec2 dirScr = s * vec2(dot(vR1, dir), dot(vR2, dir));
    vec2 dirTex = texDx * dirScr.x + texDy*dirScr.y;
    float dirTexZ = dot(nrmBaseNormal, dir);
    s = skipProj ? 1.0 : 1.0 / max(eps, abs(dirTexZ));
    return s * bumpScale * dirTex;
}

vec3 getFinalNormal(vec2 inUV, vec3 nrmBaseNormal)
{
    // TBN matrix
    vec3 vT = fragTangent;
    vec3 vB = cross(nrmBaseNormal, vT);

    // tangent space normal
    vec3 vM = textureLod(normalSampler[push.objectId], inUV, 0.0).rgb * 2.0 - 1.0;

    vec3 vMa = abs(vM);
    float z_ma = max(vMa.z, max(vMa.x, vMa.y));
    vec2 derivative = vec2(vM.x, vM.y) / z_ma;

    // calc surface gradient and final normal;
    vec3 surfGrad = derivative.x * vT + derivative.y * vB;
    return normalize(nrmBaseNormal - surfGrad);
}

vec2 parallaxMapping(vec2 uv, vec3 viewDir, int index, vec3 dPdx, vec3 dPdy, vec3 nrmBaseNormal)
{
    vec2 projV = projectVecToTextureSpace(viewDir, uv, ssbo.objects[index].heightscale, true, dPdx, dPdy, nrmBaseNormal);
    float height = textureLod(heightSampler[index], uv, 0.0).r - 0.5;
    vec2 p = height * projV;
    return uv + p;
}

vec2 parallaxOcclusionMapping(vec2 uv, vec3 viewDir, int index, vec3 dPdx, vec3 dPdy, vec3 nrmBaseNormal)
{
    vec2 projV = projectVecToTextureSpace(viewDir, uv, ssbo.objects[index].heightscale, false, dPdx, dPdy, nrmBaseNormal);
    float height = textureLod(heightSampler[index], uv, 0.0).r - 1.0;
    vec2 p = RayMarch(uv, uv + projV);
    return uv + p;
}

void main() {
    vec3 nrmBaseNormal = normalize(mat3(ssbo.objects[push.objectId].normalMatrix) * fragNormal);
    vec3 dPdx = dFdxFine(fragPosWorld.xyz);
    vec3 dPdy = dFdyFine(fragPosWorld.xyz);

    vec3 viewDirection = normalize(ubo.inverseView[3].xyz - fragPosWorld.xyz);

    vec2 uv = fragTexCoord;
    if (ssbo.objects[push.objectId].parallaxmode != 0) {
        if (ssbo.objects[push.objectId].parallaxmode == 1) {
            uv = parallaxMapping(fragTexCoord, viewDirection, push.objectId, dPdx, dPdy, nrmBaseNormal);
        } else if (ssbo.objects[push.objectId].parallaxmode == 2) {
            uv = parallaxOcclusionMapping(fragTexCoord, viewDirection, push.objectId, dPdx, dPdy, nrmBaseNormal);
        }
    }

    vec3 color = fragColor;
    if (textureQueryLevels(texSampler[push.objectId]) > 0) {
        color = texture(texSampler[push.objectId], uv).rgb;
    }

    vec3 normalHeightMapLod = normalize(mat3(ssbo.objects[push.objectId].normalMatrix) * fragNormal);
    if (textureQueryLevels(normalSampler[push.objectId]) > 0) {
        normalHeightMapLod = getFinalNormal(uv, nrmBaseNormal);
    }

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        discard;
    }

    outAlbedo = vec4(color,  0.0);
    outNormal = vec4(normalHeightMapLod, 0.0);
}
