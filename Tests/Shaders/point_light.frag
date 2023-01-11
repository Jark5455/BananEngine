#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct GameObject {
    vec4 position;
    vec4 rotation; // color for point lights
    vec4 scale; // radius for point lights

    mat4 modelMatrix;
    mat4 normalMatrix;

    int hasTexture;
    int hasNormal;

    int hasHeight;
    float heightscale;
    float parallaxBias;
    float numLayers;
    int parallaxmode;

    int isPointLight;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 inverseProjection;
    mat4 view;
    vec4 ambientLightColor;
    int numGameObjects;
} ubo;

layout(set = 0, binding = 1) readonly buffer GameObjects {
    GameObject objects[];
} ssbo;

layout(push_constant) uniform Push {
    int objectId;
} push;

const float PI = 3.1415926535897932384626433832795;

void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));

    if (dis >= 1.0) {
        discard;
    }

    float cosDis = 0.5 * (cos(dis * PI) + 1.0);
    outColor = vec4(ssbo.objects[push.objectId].rotation.xyz + cosDis, cosDis);
}