#version 450

// TODO also move this to some uniform buffer

#define SMAA_PRESET_ULTRA

#if defined(SMAA_PRESET_LOW)
#define SMAA_MAX_SEARCH_STEPS 4
#elif defined(SMAA_PRESET_MEDIUM)
#define SMAA_MAX_SEARCH_STEPS 8
#elif defined(SMAA_PRESET_HIGH)
#define SMAA_MAX_SEARCH_STEPS 16
#elif defined(SMAA_PRESET_ULTRA)
#define SMAA_MAX_SEARCH_STEPS 32
#endif

#ifndef SMAA_MAX_SEARCH_STEPS
#define SMAA_MAX_SEARCH_STEPS 16
#endif

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec2 vPixCoord;
layout(location = 2) out vec4 vOffset0;
layout(location = 3) out vec4 vOffset1;
layout(location = 4) out vec4 vOffset2;

layout(set = 1, binding = 0) uniform sampler2D edge;
layout(set = 1, binding = 1) uniform sampler2D area;
layout(set = 1, binding = 2) uniform sampler2D search;

void main() {
    vec2 resolution = textureSize(edge, 0);
    vec4 SMAA_RT_METRICS = vec4(1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y);

    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);

    vOffset0 = fma(SMAA_RT_METRICS.xyxy, vec4(-0.25, -0.125,  1.25, -0.125), outUV.xyxy);
    vOffset1 = fma(SMAA_RT_METRICS.xyxy, vec4(-0.125, -0.25, -0.125,  1.25), outUV.xyxy);
    vOffset2 = fma(SMAA_RT_METRICS.xxyy, vec4(-2.0, 2.0, -2.0, 2.0) * float(SMAA_MAX_SEARCH_STEPS), vec4(vOffset0.xz, vOffset1.yw));

    vPixCoord = outUV * SMAA_RT_METRICS.zw;
}