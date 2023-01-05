#version 450

layout(location = 0) in vec2 inUV;

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
    float heightScale;
    float parallaxBias;
    float numLayers;
    int parallaxMode;
    int numGameObjects;
} ubo;

layout(set = 0, binding = 1) readonly buffer GameObjects {
    GameObject objects[];
} ssbo;

layout(set = 1, binding = 0) uniform sampler2D normals;
layout(set = 1, binding = 1) uniform sampler2D albedo;
layout(set = 1, binding = 2) uniform sampler2D depth;

layout(location = 0) out vec4 outColor;

const float PI = 3.1415926535897932384626433832795;

vec3 reconstruct_world_position() {
    float z = textureLod(depth, inUV, 0.0).r;
    if (z == 1.0)
        discard;

    float x = inUV.x * 2.0f - 1.0f;
    float y = inUV.y * 2.0f - 1.0f;
    vec4 position_s = vec4(x, y, z, 1.0f);
    vec4 position_v =  ubo.inverseView * inverse(ubo.projection) * position_s;
    vec3 worldPos = position_v.xyz / position_v.w;
    return worldPos;
}

void main() {
    vec3 position = reconstruct_world_position();
    vec3 surfaceNormal = textureLod(normals, inUV, 0.0).rgb;
    vec3 diffuse = textureLod(albedo, inUV, 0.0).rgb;

    vec3 diffuseLight = diffuse;
    vec3 specularLight = vec3(0.0);

    vec3 viewPos = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(viewPos - position);

    for (int i = 0; i < ubo.numGameObjects; i++) {
        GameObject object = ssbo.objects[i];
        if (object.isPointLight == 1) {
            vec3 lightPos = object.position.xyz;

            vec3 directionToLight = lightPos - position;
            vec3 reflection = reflect(-directionToLight, surfaceNormal);

            float attenuation = 1.0 / dot(directionToLight, directionToLight);
            directionToLight = normalize(directionToLight);

            float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
            vec3 intensity = object.rotation.xyz * object.rotation.w * attenuation;
            diffuseLight += intensity * cosAngIncidence;

            vec3 halfAngle = normalize(directionToLight + viewDirection);
            float blinnTerm = dot(surfaceNormal, halfAngle);
            blinnTerm = clamp(blinnTerm, 0, 1);
            blinnTerm = pow(blinnTerm, 512.0f);
            specularLight += object.rotation.xyz * attenuation * blinnTerm;
        }
    }

    outColor = vec4(diffuseLight + specularLight, 1.0);
}