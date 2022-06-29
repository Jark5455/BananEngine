#version 450

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout (location = 0) out vec2 outUV;

void main() {
    gl_Position = vec4(vec3(0.0), 1.0);
}
