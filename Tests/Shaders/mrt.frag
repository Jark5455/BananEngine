#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec3 fragPos;
layout(location = 5) in vec3 fragTangent;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 shadowProjection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

layout(set = 0, binding = 1) uniform sampler2D texSampler[];

void main() {

    outPosition = vec4(inWorldPos, 1.0);
    outNormal = vec4(fragNormalWorld, 1.0);

    int index = int(push.modelMatrix[3][3]);
    if (textureQueryLevels(texSampler[index]) == 0) {
        outAlbedo = vec4(fragColor, 1.0);
    } else {
        outAlbedo = texture(texSampler[index], fragTexCoord);
    }
}