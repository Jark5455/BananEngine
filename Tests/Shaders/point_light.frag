#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 shadowProjection;
    mat4 view;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
    float heightScale;
    float parallaxBias;
    float numLayers;
} ubo;

layout (push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;
} push;

const float PI = 3.1415926535897932384626433832795;

void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));

    if (dis >= 1.0) {
        discard;
    }

    float cosDis = 0.5 * (cos(dis * PI) + 1.0);
    outColor = vec4(push.color.xyz + cosDis, cosDis);
}