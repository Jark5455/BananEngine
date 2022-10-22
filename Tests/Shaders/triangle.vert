#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 fragTangentViewPos;
layout(location = 5) out vec3 fragTangentFragPos;
layout(location = 6) out mat3 fragTBN;

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
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    mat4 actuallModelMatrix = push.modelMatrix;
    actuallModelMatrix[3][3] = 1.0;

    vec4 positionWorld = actuallModelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;

    vec3 N = normalize(mat3(actuallModelMatrix) * normal);
    vec3 T = normalize(mat3(actuallModelMatrix) * tangent);
    vec3 B = normalize(cross(N, T));
    fragTBN = transpose(mat3(T, B, N));

    fragTangentViewPos = fragTBN * ubo.inverseView[3].xyz;
    fragTangentFragPos = fragTBN * positionWorld.xyz;

    fragTexCoord = uv;
    fragColor = color;
    fragPos = position;
    fragNormal = normal;
}