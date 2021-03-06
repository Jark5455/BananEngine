#version 450

layout(location = 0) in vec3 position; //used why does glsl optimize this out
layout(location = 1) in vec3 color; //unused
layout(location = 2) in vec3 normal; //unused
layout(location = 3) in vec2 uv; //unused

layout(location = 0) out vec4 outPos;

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
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 view;
} push;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    mat4 actuallModelMatrix = push.model;
    actuallModelMatrix[3][3] = 1.0;

    gl_Position = ubo.shadowProjection * push.view * actuallModelMatrix * vec4(position, 1.0);
    outPos = actuallModelMatrix * vec4(position, 1.0);
}