#version 450

layout(location = 0) in vec4 inPos;

layout(location = 0) out float outColor;

struct PointLight {
    vec4 position;
    vec4 color;
};

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
    vec4 ambientLightColor;
    int numGameObjects;
} ubo;

layout(set = 0, binding = 1) readonly buffer GameObjects {
    GameObject objects[];
} ssbo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 viewMatrix;
} push;

void main()
{
    vec3 lightVec = inPos.xyz - ssbo.objects[0].position.xyz;
    outColor = length(lightVec);
}