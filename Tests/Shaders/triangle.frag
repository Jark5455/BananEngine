#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define EPSILON 0.15
#define SHADOW_OPACITY 0.5

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;
layout (location = 4) in vec3 fragPos;

layout (location = 0) out vec4 outColor;

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

layout(set = 1, binding = 0) uniform sampler2D texSampler[];
layout(set = 1, binding = 1) uniform sampler2D normalSampler[];
layout(set = 1, binding = 2) uniform samplerCube shadowCubeMap;


void main() {

    int index = int(push.modelMatrix[3][3]);
    vec3 diffuseLight;

    if (textureQueryLevels(texSampler[index]) == 0) {
        diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    } else {
        vec4 texture_sampler = texture(texSampler[index], fragTexCoord);
        diffuseLight = texture_sampler.rgb;
    }

    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraWorldPos = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraWorldPos - fragPosWorld);

    for (int i = 0; i < ubo.numLights; i++) {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;

        //cool reflections
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 512.0f);
        specularLight += light.color.xyz * attenuation * blinnTerm;
    }

    /*vec3 lightVec = fragPosWorld - ubo.pointLights[0].position.xyz;
    float sampledDist = texture(shadowCubeMap, lightVec).r;
    float dist = length(lightVec);

    float shadow = (dist <= sampledDist + EPSILON) ? 1.0 : SHADOW_OPACITY;*/

    outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}
