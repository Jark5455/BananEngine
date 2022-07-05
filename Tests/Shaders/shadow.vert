#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec3 outLightPos;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 viewMatrix;
} push;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.projection * push.viewMatrix * push.modelMatrix * vec4(position, 1.0);

    outPos = vec4(position, 1.0);
    outLightPos = ubo.pointLights[0].position.xyz;
}