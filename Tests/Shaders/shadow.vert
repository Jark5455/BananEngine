#version 450
#extension GL_EXT_nonuniform_qualifier : enable

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

void main()
{
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * push.viewMatrix * positionWorld;

    outPos = vec4(position, 1.0);
}