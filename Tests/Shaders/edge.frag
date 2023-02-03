#version 450

// TODO move this to uniform buffer of some sort
#ifndef SMAA_THRESHOLD
#define SMAA_THRESHOLD 0.1
#endif
#ifndef SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR
#define SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR 2.0
#endif

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 vOffset0;
layout (location = 2) in vec4 vOffset1;
layout (location = 3) in vec4 vOffset2;

layout (location = 0) out vec2 outEdges;

layout(set = 1, binding = 0) uniform sampler2D color;

void main() {
    // TODO add different modes for edge detection, currently this is luma, but add option for depth and color based edge detection

    vec2 threshold = vec2(SMAA_THRESHOLD);

    // Calculate lumas:
    vec3 weights = vec3(0.2126, 0.7152, 0.0722);
    float L = dot(texture(color, inUV).rgb, weights);

    float Lleft = dot(texture(color, vOffset0.xy).rgb, weights);
    float Ltop  = dot(texture(color, vOffset0.zw).rgb, weights);

    // We do the usual threshold:
    vec4 delta;
    delta.xy = abs(L - vec2(Lleft, Ltop));
    vec2 edges = step(threshold, delta.xy);

    // Then discard if there is no edge:
    if (dot(edges, vec2(1.0, 1.0)) == 0.0)
        discard;

    // Calculate right and bottom deltas:
    float Lright = dot(texture(color, vOffset1.xy).rgb, weights);
    float Lbottom  = dot(texture(color, vOffset1.zw).rgb, weights);
    delta.zw = abs(L - vec2(Lright, Lbottom));

    // Calculate the maximum delta in the direct neighborhood:
    vec2 maxDelta = max(delta.xy, delta.zw);

    // Calculate left-left and top-top deltas:
    float Lleftleft = dot(texture(color, vOffset2.xy).rgb, weights);
    float Ltoptop = dot(texture(color, vOffset2.zw).rgb, weights);
    delta.zw = abs(vec2(Lleft, Ltop) - vec2(Lleftleft, Ltoptop));

    // Calculate the final maximum delta:
    maxDelta = max(maxDelta.xy, delta.zw);
    float finalDelta = max(maxDelta.x, maxDelta.y);

    // Local contrast adaptation:
    edges.xy *= step(finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy);
    outEdges = edges;
}