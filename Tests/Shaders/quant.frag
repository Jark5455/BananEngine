#version 450

layout (location = 0) out vec4 outColor;

layout(set = 0, input_attachment_index = 0, binding = 0) uniform subpassInputMS depth;

void main() {
    float sum = 0.0;
    float total = 4.0;

    for (int i = 0; i < 4; i++) {
        sum += subpassLoad(depth, i).r;
    }

    float z = sum / total;
    float zNear = 0.1f;
    float zFar = 1024.f;

    float linear = zNear * zFar / (zFar + z * (zNear - zFar));

    vec4 exp = vec4(linear, pow(linear, 2), pow(linear, 3), pow(linear, 4));
    vec4 a = vec4(0.5, 0.0, 0.5, 0.0);

    const mat4 m = mat4(
        1.5, 0.0, -2.0, 0.0,
        0.0, 4.0, 0.0, -4.0,
        sqrt(3) * (1.0 / 2.0), 0, -sqrt(12) * (1.0 / 9.0), 0,
        0, 0.5, 0, 0.5
    );

    outColor = m * exp + a;
}

