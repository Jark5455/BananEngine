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
    mat4 shadowProjection;
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

vec2 parallaxMapping(vec2 uv, vec3 viewDir, int index)
{
    viewDir.y = -viewDir.y;
    float height = textureLod(heightSampler[index], uv, 0.0).r;
    vec2 p = viewDir.xy * (height * (ssbo.objects[push.objectId].heightscale * 0.5) + ssbo.objects[push.objectId].parallaxBias) / viewDir.z;
    return uv - p;
}

vec2 steepParallaxMapping(vec2 uv, vec3 viewDir, int index)
{
    viewDir.y = -viewDir.y;
    float layerDepth = 1.0 / ssbo.objects[push.objectId].numLayers;
    float currLayerDepth = 0.0;
    vec2 deltaUV = viewDir.xy * ssbo.objects[push.objectId].heightscale / (viewDir.z * ssbo.objects[push.objectId].numLayers);
    vec2 currUV = uv;
    float height = textureLod(heightSampler[index], currUV, 0.0).r;
    for (int i = 0; i < ssbo.objects[push.objectId].numLayers; i++) {
        currLayerDepth += layerDepth;
        currUV -= deltaUV;
        height = textureLod(heightSampler[index], currUV, 0.0).r;
        if (height < currLayerDepth) {
            break;
        }
    }
    return currUV;
}

vec2 parallaxOcclusionMapping(vec2 uv, vec3 viewDir, int index)
{
    viewDir.y = -viewDir.y;
    float layerDepth = 1.0 / ssbo.objects[push.objectId].numLayers;
    float currLayerDepth = 0.0;
    vec2 deltaUV = viewDir.xy * ssbo.objects[push.objectId].heightscale / (viewDir.z * ssbo.objects[push.objectId].numLayers);
    vec2 currUV = uv;
    float height = textureLod(heightSampler[index], currUV, 0.0).r;
    for (int i = 0; i < ssbo.objects[push.objectId].numLayers; i++) {
        currLayerDepth += layerDepth;
        currUV -= deltaUV;
        height = textureLod(heightSampler[index], currUV, 0.0).r;
        if (height < currLayerDepth) {
            break;
        }
    }
    vec2 prevUV = currUV + deltaUV;
    float nextDepth = height - currLayerDepth;
    float prevDepth = textureLod(heightSampler[index], prevUV, 0.0).r - currLayerDepth + layerDepth;
    return mix(currUV, prevUV, nextDepth / (nextDepth - prevDepth));
}

vec3 getFinalNormal(vec2 inUV)
{

    //************************************
    // DEFS VERY IMPORTANT
    //************************************

    vec3 relSurfPos = fragPosWorld.xyz;
    // mikkts for conventional vertex-level tangent space
    // (no normalization is mandatory). Using "bitangent on the fly"
    // option in xnormal to reduce vertex shader outputs.
    vec3 mikktsTangent = fragTangent;
    vec3 mikktsBitangent =  cross(fragNormal, fragTangent);

    // Prepare for surfgrad formulation w/o breaking mikkTSpace
    // compliance (use same scale as interpolated vertex normal).
    float renormFactor = 1.0 / length(fragNormal);
    mikktsTangent *= renormFactor;
    mikktsBitangent *= renormFactor;
    vec3 nrmBaseNormal = renormFactor * mat3(ssbo.objects[push.objectId].normalMatrix) * fragNormal;
    // The variables below (plus nrmBaseNormal) need to be
    // recomputed in the case of post-resolve bump mapping.
    vec3 dPdx = dFdxFine(relSurfPos);
    vec3 dPdy = dFdyFine(relSurfPos);
    vec3 sigmaX = dPdx - dot(dPdx, nrmBaseNormal) * nrmBaseNormal;
    vec3 sigmaY = dPdy - dot(dPdy, nrmBaseNormal) * nrmBaseNormal;
    float flip_sign = dot(dPdy, cross(nrmBaseNormal, dPdx)) < 0 ? -1 : 1;

    //************************************
    // GEN BASIS TB FUNCTION VERY IMPORTANT
    //************************************

    vec2 dSTdx = dFdxFine(inUV);
    vec2 dSTdy = dFdyFine(inUV);
    float det = dot(dSTdx, vec2(dSTdy.y, -dSTdy.x));
    float sign_det = det < 0.0 ? -1.0 : 1.0;
    // invC0 represents (dXds, dYds), but we don’t divide
    // by the determinant. Instead, we scale by the sign.
    vec2 invC0 = sign_det * vec2(dSTdy.y, -dSTdx.y);
    vec3 vT = sigmaX * invC0.x + sigmaY * invC0.y;
    if (abs(det) > 0.0)
    vT = normalize(vT);

    vec3 vB = (sign_det * flip_sign) * cross(nrmBaseNormal, vT);

    vec3 vM = textureLod(normalSampler[push.objectId], inUV, 0.0).rgb * 2.0 - 1.0;

    //************************************
    // GEN DERIV FROM TBN
    //************************************

    float scale = 1.0 / 128.0;
    // Ensure vM delivers a positive third component using abs() and
    // constrain vM.z so the range of the derivative is [-128; 128].
    vec3 vMa = abs(vM);
    float z_ma = max(vMa.z, scale * max(vMa.x, vMa.y));
    // Set to match positive vertical texture coordinate axis.
    bool gFlipVertDeriv = false;
    float s = gFlipVertDeriv ? -1.0 : 1.0;
    vec2 derivative = vec2(vM.x, s * vM.y) / z_ma;

    //************************************
    // FINAL CALC
    //************************************

    vec3 surfGrad = derivative.x * vT + derivative.y * vB;
    return normalize(nrmBaseNormal - surfGrad);
}

void main() {

    vec3 N = normalize(mat3(ssbo.objects[push.objectId].modelMatrix) * fragNormal);
    vec3 T = normalize(mat3(ssbo.objects[push.objectId].modelMatrix) * fragTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(cross(N, T));
    mat3 fragTBN = transpose(mat3(T, B, N));

    vec3 tangentViewPos = fragTBN * ubo.inverseView[3].xyz;
    vec3 tangentFragPos = fragTBN * vec3(fragPosWorld);

    vec3 viewDirection = normalize(tangentViewPos - tangentFragPos);

    vec2 uv = fragTexCoord;
    if (ssbo.objects[push.objectId].parallaxmode != 0) {
        if (ssbo.objects[push.objectId].parallaxmode == 1) {
            uv = parallaxMapping(fragTexCoord, viewDirection, push.objectId);
        }  else if (ssbo.objects[push.objectId].parallaxmode == 2) {
            uv = steepParallaxMapping(fragTexCoord, viewDirection, push.objectId);
        } else if (ssbo.objects[push.objectId].parallaxmode == 3) {
            uv = parallaxOcclusionMapping(fragTexCoord, viewDirection, push.objectId);
        }
    }

    vec3 color = fragColor;
    if (textureQueryLevels(texSampler[push.objectId]) > 0) {
        color = texture(texSampler[push.objectId], uv).rgb;
    }

    vec3 normalHeightMapLod = normalize(mat3(ssbo.objects[push.objectId].normalMatrix) * fragNormal);
    if (textureQueryLevels(normalSampler[push.objectId]) > 0) {
        normalHeightMapLod = getFinalNormal(uv);
    }

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        discard;
    }

    outAlbedo = vec4(color,  uv.x);
    outNormal = vec4(normalHeightMapLod, uv.y);
}