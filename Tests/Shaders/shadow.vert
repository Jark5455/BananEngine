#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec2 uv;

layout(location = 0) out vec4 outPos;

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
    mat4 shadowProjection;
    mat4 view;
    mat4 inverseView;
    int numGameObjects;
} ubo;

layout(set = 0, binding = 1) readonly buffer GameObjects {
    GameObject objects[];
} ssbo;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 view;
} push;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    mat4 actuallModelMatrix = push.model;
    actuallModelMatrix[3][3] = 1.0;

    gl_Position = ubo.shadowProjection * push.view * actuallModelMatrix * vec4(position, 1.0);
    outPos = actuallModelMatrix * vec4(position, 1.0);
}