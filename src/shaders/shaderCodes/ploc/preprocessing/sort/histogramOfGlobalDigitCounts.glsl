#version 460 core

// constants
const uint NB_BITS = 32; // k = 32
const uint NB_DIGIT = 4; // d = 4
const uint NB_DIGIT_PLACE = 8; // p = sup(k/d)
const uint NB_ITEMS_PER_THREAD = 8;

// inputs
const uint LOCAL_SIZE_X = 16;
const uint LOCAL_SIZE_Y = 16;
layout(local_size_x = LOCAL_SIZE_X, local_size_y = LOCAL_SIZE_Y) in;

layout (binding = 2, std430) readonly buffer uValuesToSortSSBO {
    uint uValuesToSort[];
};

uniform uint uNbValuesToSort;


// outputs
layout (binding = 3, std430) buffer uGlobalHistogramSSBO {
    uint uGlobalHistogram[];
};


// shared items
shared uint sHistogramTile[NB_DIGIT*NB_DIGIT_PLACE];

// functions
void main(){
    uint instanceIndex = gl_GlobalInvocationID.x * NB_ITEMS_PER_THREAD 
        + gl_GlobalInvocationID.y * (gl_NumWorkGroups.x * gl_WorkGroupSize.x * NB_ITEMS_PER_THREAD);

    // feed shared histogram
    for(uint i=instanceIndex; i<(instanceIndex+NB_ITEMS_PER_THREAD); i++){
        if(i >= uNbValuesToSort){break;}
        for(uint k=0; k<NB_DIGIT_PLACE; k++){
            for(uint j=0; j<NB_DIGIT; j++){
                uint bitPosition = (k*NB_DIGIT)+j;
                uint mask = uint(1) << bitPosition;
                uint currentValue = uValuesToSort[i];
                if((currentValue & mask) != (uint(0))) {
                    atomicAdd(sHistogramTile[bitPosition], 1);
                }
            }
        }
    }

    // add shared histogram to global histogram
    barrier();
    memoryBarrierShared();
    // only one thread per work group performs the copy
    if (gl_LocalInvocationIndex == 0) { 
        for(uint i = 0; i < NB_DIGIT * NB_DIGIT_PLACE; i++) {
            atomicAdd(uGlobalHistogram[i], sHistogramTile[i]);
            // atomicAdd(uGlobalHistogram[i], 1);
        }
    }
}