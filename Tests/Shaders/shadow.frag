#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_multiview : require
// dummy shader does nothing since we are only using depth anyways

layout (location = 0) in vec4 inPos;
layout (location = 0) out float outColor;


layout(set = 2, binding = 0) uniform ShadowViews {
    mat4 projectionMatrix;
    mat4 viewMatrices[6];
    mat4 invViewMatrices[6];
} mats;

void main() {
    vec3 lightVec = inPos.xyz - mats.viewMatrices[gl_ViewIndex][3].xyz;
    outColor = length(lightVec);
}