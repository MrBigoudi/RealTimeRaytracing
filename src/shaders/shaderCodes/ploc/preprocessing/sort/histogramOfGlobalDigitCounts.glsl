#version 460 core

// constants
const uint NB_BITS = 30; // k = 30
const uint NB_DIGIT = 8; // d = 8
const uint NB_DIGIT_PLACE = 4; // p = sup(k/d)
const uint NB_ITEMS_PER_THREAD = 8;

// inputs
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout (binding = 2, std430) readonly buffer uValuesToSort {
    uint uValuesToSort[];
};

uniform uint uNbValuesToSort;


// outputs
layout (binding = 3, std430) buffer uGlobalHistogram {
    uint uGlobalHistogram[];
};


// shared items
shared uint sHistogramTile[NB_DIGIT*NB_DIGIT_PLACE];

// functions
void main(){
    uint instanceIndex = gl_GlobalInvocationID.x*NB_ITEMS_PER_THREAD;

    // feed shared histogram
    for(uint i=instanceIndex; i<(instanceIndex+NB_ITEMS_PER_THREAD); i++){
        if(i >= uNbValuesToSort){break;}
        for(uint k=0; k<NB_DIGIT_PLACE; k++){
            for(uint j=0; j<NB_DIGIT; j++){
                uint bitPosition = (k*NB_DIGIT)+j;
                if (uValuesToSort[i] & (1 << bitPosition)) {
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
        }
    }
}