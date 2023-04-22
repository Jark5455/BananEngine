#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_multiview : require

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outColor;

layout(set = 0, input_attachment_index = 0, binding = 0) uniform subpassInputMS depth;

layout(set = 1, binding = 0) uniform ShadowViews {
    mat4 projectionMatrix;
    mat4 invProjection;
    mat4 viewMatrices[6];
    mat4 invViewMatrices[6];
} mats;

// TODO DO NOT USE THIS
// POSSIBLY THE SLOWEST WAY TO RECONSTRUCT WORLD DEPTH LMAO
vec3 reconstruct_view_position(float z)
{
    float x = inUV.x * 2.0f - 1.0f;
    float y = inUV.y * 2.0f - 1.0f;
    vec4 position_s = vec4(x, y, z, 1.0f);
    vec4 position_v = mats.invProjection * position_s;
    return position_v.xyz / position_v.w;
}

void main() {
    float sum = 0.0;
    float total = 4.0;

    for (int i = 0; i < 4; i++) {
        sum += subpassLoad(depth, i).r;
    }

    float z = sum / total;
    float linear = length(reconstruct_view_position(z));

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

