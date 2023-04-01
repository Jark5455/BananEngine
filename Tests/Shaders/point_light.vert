#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

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

layout(buffer_reference, std430) buffer pointLight {
    vec4 position;
    vec4 color;
    float intensity;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 inverseProjection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    int numGameObjects;
} ubo;

layout(set = 1, binding = 0) uniform GameObjects {
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

void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
    vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

    vec3 positionWorld = ssbo.objects[push.objectId].position.xyz + ssbo.objects[push.objectId].scale.r * fragOffset.x * cameraRightWorld + ssbo.objects[push.objectId].scale.r * fragOffset.y * cameraUpWorld;
    gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}