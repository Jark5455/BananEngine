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
  vec4 ambientLightColor;
  float heightScale;
  float parallaxBias;
  float numLayers;
  int numGameObjects;
} ubo;

layout(set = 0, binding = 1) readonly buffer GameObjects {
  GameObject objects[];
} ssbo;

layout(push_constant) uniform Push {
  int objectId;
} push;

void main() {
  fragOffset = OFFSETS[gl_VertexIndex];
  vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
  vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

  vec3 positionWorld = ssbo.objects[push.objectId].position.xyz + ssbo.objects[push.objectId].scale.r * fragOffset.x * cameraRightWorld + ssbo.objects[push.objectId].scale.r * fragOffset.y * cameraUpWorld;
  gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}