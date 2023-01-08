#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragPos;
layout (location = 3) out vec3 fragNormal;
layout (location = 4) out vec4 fragPosWorld;
layout (location = 5) out vec3 fragTangent;

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
    PointLight pointLights[10];
    int numLights;
    float heightScale;
    float parallaxBias;
    float numLayers;
} ubo;

layout(set = 0, binding = 1) readonly buffer GameObjects {
    GameObject objects[];
} ssbo;

layout(push_constant) uniform Push {
    int objectId;
} push;

void main() {
    fragPosWorld = ssbo.objects[push.objectId].modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * fragPosWorld;

    fragTexCoord = uv;
    fragColor = color;
    fragPos = position;
    fragNormal = normal;
    fragTangent = tangent;
}
