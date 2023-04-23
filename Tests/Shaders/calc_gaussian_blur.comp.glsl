#version 120

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform restrict readonly image2D u_input_image;
layout(rgba8, binding = 1) uniform restrict writeonly image2D u_output_image;

const int M = 16;
const int N = 2 * M + 1;


void main() {

}
