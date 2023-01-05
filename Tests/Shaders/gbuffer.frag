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
    float heightScale;
    float parallaxBias;
    float numLayers;
    int parallaxMode;
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
    vec2 p = viewDir.xy * (height * (ubo.heightScale * 0.5) + ubo.parallaxBias) / viewDir.z;
    return uv - p;
}

vec2 steepParallaxMapping(vec2 uv, vec3 viewDir, int index)
{
    viewDir.y = -viewDir.y;
    float layerDepth = 1.0 / ubo.numLayers;
    float currLayerDepth = 0.0;
    vec2 deltaUV = viewDir.xy * ubo.heightScale / (viewDir.z * ubo.numLayers);
    vec2 currUV = uv;
    float height = textureLod(heightSampler[index], currUV, 0.0).r;
    for (int i = 0; i < ubo.numLayers; i++) {
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
    float layerDepth = 1.0 / ubo.numLayers;
    float currLayerDepth = 0.0;
    vec2 deltaUV = viewDir.xy * ubo.heightScale / (viewDir.z * ubo.numLayers);
    vec2 currUV = uv;
    float height = textureLod(heightSampler[index], currUV, 0.0).r;
    for (int i = 0; i < ubo.numLayers; i++) {
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

void main() {
    vec3 tangentViewPos = fragTBN * ubo.inverseView[3].xyz;
    vec3 tangentFragPos = fragTBN * vec3(fragPosWorld);

    vec3 viewDirection = normalize(tangentViewPos - tangentFragPos);

    vec2 uv = fragTexCoord;
    if (textureQueryLevels(heightSampler[push.objectId]) > 0) {
        uv = parallaxOcclusionMapping(fragTexCoord, viewDirection, push.objectId);
    }

    vec3 color = fragColor;
    if (textureQueryLevels(texSampler[push.objectId]) > 0) {
        color = texture(texSampler[push.objectId], uv).rgb;
    }

    vec3 normalHeightMapLod = normalize(mat3(ssbo.objects[push.objectId].normalMatrix) * fragNormal);
    if (textureQueryLevels(normalSampler[push.objectId]) > 0) {
        normalHeightMapLod = textureLod(normalSampler[push.objectId], uv, 0.0).rgb;
    }

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        discard;
    }

    // Compute the surface gradients at each vertex
    vec3 dpdu = vec3(dFdxFine(fragPos));
    vec3 dpdv = vec3(dFdyFine(fragPos));

    // Interpolate the gradients across the surface of the triangle
    vec3 dpduInterp = mix(dpdu, dpdu, uv.y);
    vec3 dpdvInterp = mix(dpdv, dpdv, uv.x);

    // Use the gradients to perturb the surface normal
    vec3 perturbedNormal = normalHeightMapLod + dpduInterp + dpdvInterp;

    // Transform the perturbed normal into view space
    vec3 viewSpaceNormal = mat3(ssbo.objects[push.objectId].normalMatrix) * perturbedNormal;

    outAlbedo = vec4(color, 1.0);
    outNormal = vec4(normalize(viewSpaceNormal), 1.0);
}