#version 450

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 vOffset0;
layout(location = 2) out vec4 vOffset1;
layout(location = 3) out vec4 vOffset2;

layout(set = 1, binding = 0) uniform sampler2D color;

void main() {
    vec2 resolution = textureSize(color, 0);
    vec4 SMAA_RT_METRICS = vec4(1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y);

    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);

    vOffset0 = fma(SMAA_RT_METRICS.xyxy, vec4(-1.0, 0.0, 0.0, -1.0), outUV.xyxy);
    vOffset1 = fma(SMAA_RT_METRICS.xyxy, vec4( 1.0, 0.0, 0.0,  1.0), outUV.xyxy);
    vOffset2 = fma(SMAA_RT_METRICS.xyxy, vec4(-2.0, 0.0, 0.0, -2.0), outUV.xyxy);
}