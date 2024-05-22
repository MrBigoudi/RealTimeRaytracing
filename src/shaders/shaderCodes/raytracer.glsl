#version 460 core

// structures
struct Camera {
    mat4 _View;
    mat4 _Proj;
    vec4 _Eye;
    float _PlaneWidth;
    float _PlaneHeight;
};

// output
layout(rgba32f, binding = 0) uniform image2D oImage;

// input
layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;
layout (location = 0) uniform float uTime;
layout (location = 1) uniform Camera uCamera;


// code
void main() {
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    float speed = 100;
    float width = 1000;
    float rd = sin(uTime*(uCamera._View[0][0] + uCamera._PlaneWidth * uCamera._PlaneHeight));

    value.x = mod(float(texelCoord.x) + uTime * speed, width) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
    value.y = float(texelCoord.y)/(gl_NumWorkGroups.y * gl_WorkGroupSize.y);
    imageStore(oImage, texelCoord, rd*value);
}