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
    
}