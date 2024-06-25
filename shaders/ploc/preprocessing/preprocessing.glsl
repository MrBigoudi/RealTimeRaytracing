#version 460 core

// input
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform vec3 uCircumscribedCubeMin;
uniform vec3 uCircumscribedCubeMax;
uniform uint uNbTriangles;

struct Triangle {
    vec4 _P0;
    vec4 _P2;
    vec4 _P1;
    uint _ModelId;
};

layout (binding = 3, std430) readonly buffer uTrianglesSSBO {
    Triangle uTriangles[];
};

// output

// functions
uint getMortonCode(uint index){}


void main(){
    uvec3 globalID = gl_GlobalInvocationID;
    uint instanceIndex = globalID.x
                        + globalID.y * gl_NumWorkGroups.x;
                        + globalID.z * gl_NumWorkGroups.x * gl_NumWorkGroups.y;

    
}