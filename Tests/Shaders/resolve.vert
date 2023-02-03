#version 450

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 vOffset;

layout(set = 1, binding = 0) uniform sampler2D color;
layout(set = 1, binding = 1) uniform sampler2D weight;

void main() {
    vec2 resolution = textureSize(color, 0);
    vec4 SMAA_RT_METRICS = vec4(1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y);

    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);

    vOffset = fma(SMAA_RT_METRICS.xyxy, vec4(1.0, 0.0, 0.0,  1.0), outUV.xyxy);
}