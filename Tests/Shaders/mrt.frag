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

vec3 calculate_view_position(vec2 texture_coordinate, float depth_from_depth_buffer)
{
    mat4 inverse_projection_matrix = inverse(ubo.projection);
    vec3 clip_space_position = vec3(texture_coordinate, depth_from_depth_buffer) * 2.0 - vec3(1.0);
    vec4 view_position = vec4(vec2(inverse_projection_matrix[0][0], inverse_projection_matrix[1][1]) * clip_space_position.xy, inverse_projection_matrix[2][3] * clip_space_position.z + inverse_projection_matrix[3][3]);
    return(view_position.xyz / view_position.w);
}

void main() {

    vec3 position = calculate_view_position(inUV, texture(depth, inUV).r) * ubo.inverseView;
    vec3 normal = normalize(vec3(texture(normals, inUV)) * 2.0 - 1.0);
    vec3 diffuse = vec3(texture(albedo, inUV));

    vec3 diffuseLight = diffuse;
    vec3 specularLight = vec3(0.0);

    vec3 viewPos = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(viewPos - position);

    for (int i = 0; i < ubo.numGameObjects; i++) {
        GameObject object = ssbo.objects[i];
        if (object.isPointLight == 1) {
            vec3 lightPos = object.position.xyz;

            vec3 directionToLight = lightPos - position;
            vec3 reflection = reflect(-directionToLight, normal);

            float attenuation = 1.0 / dot(directionToLight, directionToLight);
            directionToLight = normalize(directionToLight);

            float cosAngIncidence = max(dot(normal, normalize(directionToLight)), 0);
            vec3 intensity = object.rotation.xyz * object.rotation.w * attenuation;
            diffuseLight += intensity * cosAngIncidence;

            vec3 halfAngle = normalize(directionToLight + viewDirection);
            float blinnTerm = dot(normal, halfAngle);
            blinnTerm = clamp(blinnTerm, 0, 1);
            blinnTerm = pow(blinnTerm, 512.0f);
            specularLight += object.rotation.xyz * attenuation * blinnTerm;
        }
    }

    outColor = vec4(diffuseLight + specularLight, 1.0);
}