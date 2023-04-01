#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require

layout(location = 0) in vec2 inUV;

layout(buffer_reference, std430) buffer transform {
    mat4 modelMatrix;
    mat4 normalMatrix;
};

layout(buffer_reference, std430) buffer parallax {
    float heightscale;
    float parallaxBias;
    float numLayers;
    int parallaxmode;
};

layout(buffer_reference) buffer pointLight;
layout(buffer_reference, std430) buffer pointLight {
    vec4 position;
    vec4 color;
    float radius;
    float intensity;

    int hasNext;
    pointLight next;
};

layout(set = 2, binding = 0) uniform GameObjects {
    int albedoTexture;
    int normalTexture;
    int heightTexture;

    int transform;
    transform transformRef;

    int parallax;
    parallax parallaxRef;

    int pointLight;
    pointLight pointLightRef;
} objectData;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 inverseProjection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    int numGameObjects;
    int numPointLights;
    pointLight basePointLightRef;
} ubo;

layout(set = 1, input_attachment_index = 0, binding = 0) uniform subpassInput depth;
layout(set = 1, input_attachment_index = 1, binding = 1) uniform subpassInput normals;
layout(set = 1, input_attachment_index = 2, binding = 2) uniform subpassInput albedo;

layout(location = 0) out vec4 outColor;

const float PI = 3.1415926535897932384626433832795;

vec3 reconstruct_world_position()
{
    float z = subpassLoad(depth).r;
    if (z == 1.0)
        discard;

    float x = inUV.x * 2.0f - 1.0f;
    float y = inUV.y * 2.0f - 1.0f;
    vec4 position_s = vec4(x, y, z, 1.0f);
    vec4 position_v =  ubo.inverseView * ubo.inverseProjection * position_s;
    vec3 worldPos = position_v.xyz / position_v.w;
    return worldPos;
}

void main() {
    vec3 position = reconstruct_world_position();
    vec3 surfaceNormal = subpassLoad(normals).rgb;
    vec3 diffuse = subpassLoad(albedo).rgb;

    vec3 diffuseLight = diffuse;
    vec3 specularLight = vec3(0.0);

    vec3 viewPos = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(viewPos - position);

    if (ubo.numPointLights > 0) {
        pointLight object = ubo.basePointLightRef;

        do {
            vec3 lightPos = object.position.xyz;

            vec3 directionToLight = lightPos - position;
            vec3 reflection = reflect(-directionToLight, surfaceNormal);

            float attenuation = 1.0 / dot(directionToLight, directionToLight);
            directionToLight = normalize(directionToLight);

            float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
            vec3 intensity = object.color.xyz * object.color.w * attenuation;
            diffuseLight += intensity * cosAngIncidence;

            vec3 halfAngle = normalize(directionToLight + viewDirection);
            float blinnTerm = dot(surfaceNormal, halfAngle);
            blinnTerm = clamp(blinnTerm, 0, 1);
            blinnTerm = pow(blinnTerm, 512.0f);
            specularLight += object.color.xyz * attenuation * blinnTerm;

            object = object.next;

        } while (object.hasNext == 1);
    }

    outColor = vec4(diffuseLight + specularLight, 1.0);
}