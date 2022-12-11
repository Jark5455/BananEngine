#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define EPSILON 0.15
#define SHADOW_OPACITY 0.5

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 fragPos;
layout (location = 3) in vec3 fragNormal;
layout (location = 4) in vec4 fragPosWorld;
layout (location = 5) in mat3 fragTBN;

layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 shadowProjection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
    float heightScale;
    float parallaxBias;
    float numLayers;
    int parallaxMode;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];
//layout(set = 1, binding = 1) uniform samplerCube shadowCubeMap;
layout(set = 2, binding = 0) uniform sampler2D normalSampler[];
layout(set = 3, binding = 0) uniform sampler2D heightSampler[];

vec2 parallaxMapping(vec2 uv, vec3 viewDir, int index)
{
    float height = 1.0 - textureLod(heightSampler[index], uv, 0.0).r;
    vec2 p = viewDir.xy * (height * (ubo.heightScale * 0.5) + ubo.parallaxBias) / viewDir.z;
    return uv - p;
}

vec2 steepParallaxMapping(vec2 uv, vec3 viewDir, int index)
{
    float layerDepth = 1.0 / ubo.numLayers;
    float currLayerDepth = 0.0;
    vec2 deltaUV = viewDir.xy * ubo.heightScale / (viewDir.z * ubo.numLayers);
    vec2 currUV = uv;
    float height = 1.0 - textureLod(heightSampler[index], currUV, 0.0).r;
    for (int i = 0; i < ubo.numLayers; i++) {
        currLayerDepth += layerDepth;
        currUV -= deltaUV;
        height = 1.0 - textureLod(heightSampler[index], currUV, 0.0).r;
        if (height < currLayerDepth) {
            break;
        }
    }
    return currUV;
}

vec2 parallaxOcclusionMapping(vec2 uv, vec3 viewDir, int index)
{
    float layerDepth = 1.0 / ubo.numLayers;
    float currLayerDepth = 0.0;
    vec2 deltaUV = viewDir.xy * ubo.heightScale / (viewDir.z * ubo.numLayers);
    vec2 currUV = uv;
    float height = 1.0 - textureLod(heightSampler[index], currUV, 0.0).r;
    for (int i = 0; i < ubo.numLayers; i++) {
        currLayerDepth += layerDepth;
        currUV -= deltaUV;
        height = 1.0 - textureLod(heightSampler[index], currUV, 0.0).r;
        if (height < currLayerDepth) {
            break;
        }
    }
    vec2 prevUV = currUV + deltaUV;
    float nextDepth = height - currLayerDepth;
    float prevDepth = 1.0 - textureLod(heightSampler[index], prevUV, 0.0).r - currLayerDepth + layerDepth;
    return mix(currUV, prevUV, nextDepth / (nextDepth - prevDepth));
}

void main() {
    int index = int(push.modelMatrix[3][3]);

    vec3 diffuseLight = vec3(0.0);
    vec3 specularLight = vec3(0.0);

    vec3 tangentViewPos = fragTBN * ubo.inverseView[3].xyz;
    vec3 tangentFragPos = fragTBN * vec3(fragPosWorld);

    vec3 viewDirection = normalize(tangentViewPos - tangentFragPos);

    // TODO it definetly doesnt work
    vec2 uv = fragTexCoord;
    if (textureQueryLevels(heightSampler[index]) > 0) {
        vec2 uv =  parallaxOcclusionMapping(fragTexCoord, viewDirection, index);
    }

    vec3 color = fragColor;
    if (textureQueryLevels(texSampler[index]) > 0) {
        color = texture(texSampler[index], uv).rgb;
    }

    vec3 normalHeightMapLod = fragTBN * normalize(mat3(push.normalMatrix) * fragNormal);
    if (textureQueryLevels(normalSampler[index]) > 0) {
        normalHeightMapLod = textureLod(normalSampler[index], uv, 0.0).rgb;
    }

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        discard;
    }

    vec3 surfaceNormal = normalize(normalHeightMapLod * 2.0 - 1.0);
    for (int i = 0; i < ubo.numLights; i++) {
        PointLight light = ubo.pointLights[i];
        vec3 tangentLightPos = fragTBN * light.position.xyz;

        vec3 directionToLight = tangentLightPos - tangentFragPos;
        vec3 reflection = reflect(-directionToLight, surfaceNormal);

        float attenuation = 1.0 / dot(directionToLight, directionToLight);
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;
        diffuseLight += intensity * cosAngIncidence;

        //cool reflections
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 512.0f);
        specularLight += light.color.xyz * attenuation * blinnTerm;
    }

    diffuseLight += color;

    /*vec3 lightVec = fragPosWorld - ubo.pointLights[0].position.xyz;
    float sampledDist = texture(shadowCubeMap, lightVec).r;
    float dist = length(lightVec);
    float shadow = (dist <= sampledDist + EPSILON) ? 1.0 : SHADOW_OPACITY;*/

    outColor = vec4(diffuseLight + specularLight, 1.0);
}
