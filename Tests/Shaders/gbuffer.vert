#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require


layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragPos;
layout (location = 3) out vec3 fragNormal;
layout (location = 4) out vec4 fragPosWorld;
layout (location = 5) out vec3 fragTangent;

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

layout(set = 2, binding = 0) uniform GameObjects {
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
    fragPosWorld = objectData.transformRef.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * fragPosWorld;

    fragTexCoord = uv;
    fragColor = color;
    fragPos = position;
    fragNormal = normal;
    fragTangent = tangent;
}
