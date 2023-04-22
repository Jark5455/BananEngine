#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_multiview : require

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outColor;

layout(set = 0, input_attachment_index = 0, binding = 0) uniform subpassInputMS depth;

void main() {
    float sum = 0.0;
    float total = 4.0;

    for (int i = 0; i < 4; i++) {
        sum += subpassLoad(depth, i).r;
    }

    float z = sum / total;

    if (z == 1.0) {
        discard;
    }

    float unnormalized = z * 2.0 - 1.0;
    vec4 exp = vec4(unnormalized, pow(unnormalized, 2), pow(unnormalized, 3), pow(unnormalized, 4));
    vec4 a = vec4(0.5, 0.0, 0.5, 0.0);

    const mat4 m = mat4(
        1.5, 0.0, -2.0, 0.0,
        0.0, 4.0, 0.0, -4.0,
        sqrt(3.0 / 4.0), 0, -sqrt(12.0 / 81.0), 0.0,
        0.0, 0.5, 0.0, 0.5
    );

    outColor = exp * m + a;
}

