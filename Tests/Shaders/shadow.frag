#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_multiview : require

layout (location = 0) in vec4 fragPosWorld;
layout (location = 0) out vec4 outColor;

layout(buffer_reference, std430) buffer transform {
    mat4 modelMatrix;
    mat4 normalMatrix;
};

layout(buffer_reference, std430) buffer parallax {
    float heightscale;
    float parallaxBias;
    float numLayers;
    int parallaxmode;
};

layout(buffer_reference) buffer pointLight;
layout(buffer_reference, std430) buffer pointLight {
    vec4 position;
    vec4 color;
    float radius;
    float intensity;

    int hasNext;
    pointLight next;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 inverseProjection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    int numGameObjects;
    int numPointLights;
    pointLight basePointLightRef;
} ubo;

layout(set = 1, binding = 0) uniform GameObjects {
    int albedoTexture;
    int normalTexture;
    int heightTexture;

    int transform;
    transform transformRef;

    int parallax;
    parallax parallaxRef;

    int pointLight;
    pointLight pointLightRef;
} objectData;

layout(set = 2, binding = 0) uniform ShadowViews {
    mat4 projectionMatrix;
    mat4 viewMatrices[6];
    mat4 invViewMatrices[6];
} mats;

void main() {
    float z = length(fragPosWorld.xyz - mats.invViewMatrices[gl_ViewIndex][3].xyz);
    outColor = vec4(z, pow(z, 2), pow(z, 3), pow(z, 4));
}