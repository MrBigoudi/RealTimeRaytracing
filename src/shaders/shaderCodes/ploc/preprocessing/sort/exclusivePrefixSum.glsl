#version 460 core

// constants
const uint NB_BITS = 30; // k = 30
const uint NB_DIGIT = 8; // d = 8
const uint NB_DIGIT_PLACE = 4; // p = sup(k/d)
const uint NB_ITEMS_PER_THREAD = 8;

// inputs
layout(local_size_x = NB_DIGIT) in;

layout (binding = 2, std430) buffer uGlobalHistogram {
    uint uGlobalHistogram[];
};

// outputs
layout (binding = 3, std430) buffer uGlobalHistogramScan {
    uint uGlobalHistogramScan[];
};

shared uint sLocalHistogramScan[NB_DIGIT*NB_DIGIT_PLACE];

// functions
void main(){
    uint id = gl_LocalInvocationIndex.x;

    // each invocations is responsible for the content of
    // two elements of the output array
    uint digitPlace = gl_GlobalInvocationID.x;
    const uint histogramFirstValue = digitPlace * NB_DIGIT;
    sLocalHistogramScan[id*2] = uGlobalHistogram[histogramFirstValue + id*2];
    sLocalHistogramScan[id*2+1] = uGlobalHistogram[histogramFirstValue + id*2+1];

    // sync to make sure that everyone has initialized 
    // their elements of shared data with data loaded from 
    // the input
    barrier();
    memoryBarrierShared();

    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;
    uint rd_id = 0;
    uint wr_id = 0;
    uint mask = 0;
    for(uint i = 0; i<steps; i++){
        // get read and write index
        mask = (1 << i) - 1;
        rd_id = ((id >> i) << (i + 1)) + mask;
        wr_id = rd_id + 1 + (id & mask);

        // accumulate the read data into our element
        sLocalHistogramScan[wr_id] += sLocalHistogramScan[rd_id];
        
        // sync 
        barrier();
        memoryBarrierShared();
    }

    // write data back to the output
    uGlobalHistogramScan[histogramFirstValue + id*2] = sLocalHistogramScan[id*2];
    uGlobalHistogramScan[histogramFirstValue + id*2+1] = sLocalHistogramScan[id*2+1];
}