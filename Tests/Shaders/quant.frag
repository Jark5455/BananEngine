#version 450

layout (location = 0) in vec2 outUV;
layout (location = 0) out vec4 outColor;

layout(set = 0, input_attachment_index = 0, binding = 0) uniform subpassInput depth;

void main() {
    float z = subpassLoad(depth).r;
    vec4 d = vec4(z, pow(z, 2), pow(z, 3), pow(z, 4));

    const mat4 m = mat4(
        1.5, 0.0, -2.0, 0.0,
        0.0, 4.0, 0.0, -4.0,
        sqrt(3) * (1.0 / 2.0), 0, -sqrt(12) * (1.0 / 9.0), 0,
        0, 0.5, 0, 0.5
    );

    vec4 a = vec4(0.0, 0.5, 0.0, 0.5);

    outColor = m * d + a;
}

