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

vec3 positionFromDepth(vec2 uv) {
    // Get the depth value for this pixel
    float z = texture(depth, uv).r;
    // Get x/w and y/w from the viewport position
    float x = uv.x * 2 - 1;
    float y = (1 - uv.y) * 2 - 1;
    vec4 projectedPosition = vec4(x, y, z, 1.0);
    // Transform by the inverse projection matrix
    vec4 positionVS = projectedPosition * inverse(ubo.projection);
    // Divide by w to get the view-space position
    return positionVS.xyz / positionVS.w;
}

void main() {
    vec3 position = positionFromDepth(inUV);
    vec3 normal = normalize(vec3(texture(normals, inUV)) * 2.0 - 1.0);
    vec4 diffuse = texture(albedo, inUV);

    vec3 diffuseLight = vec3(diffuse);
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

            //cool reflections
            vec3 halfAngle = normalize(directionToLight + viewDirection);
            float blinnTerm = dot(normal, halfAngle);
            blinnTerm = clamp(blinnTerm, 0, 1);
            blinnTerm = pow(blinnTerm, 512.0f);
            specularLight += object.rotation.xyz * attenuation * blinnTerm;
        }
    }
}