#version 450

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;


void main() {
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
