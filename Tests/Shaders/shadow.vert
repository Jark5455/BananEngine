#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_multiview : require

layout (location = 0) in vec3 position;
layout (location = 0) out vec4 fragPosWorld;

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
    fragPosWorld = objectData.transformRef.modelMatrix * vec4(position, 1.0);
    gl_Position = mats.projectionMatrix * mats.viewMatrices[gl_ViewIndex] * fragPosWorld;
}