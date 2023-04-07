#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require

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

void main() {

    const mat4 m = mat4(
        1.5, 0.0, -2.0, 0.0,
        0.0, 4.0, 0.0, -4.0,
        sqrt(3) * (1.0 / 2.0), 0, -sqrt(12) * (1.0 / 9.0), 0,
        0, 0.5, 0, 0.5
    );

    vec4 z = vec4(gl_FragCoord.z, pow(gl_FragCoord.z, 2), pow(gl_FragCoord.z, 3), pow(gl_FragCoord.z, 4));
    vec4 a = vec4(0.0, 0.5, 0.0, 0.5);

    outColor = m * z + a;
}