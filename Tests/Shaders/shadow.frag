#version 450

layout(location = 0) in vec4 inPos;

layout(location = 0) out float outColor;

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
    vec3 lightVec = inPos.xyz - ubo.pointLights[0].position.xyz;
    outColor = length(lightVec);
}