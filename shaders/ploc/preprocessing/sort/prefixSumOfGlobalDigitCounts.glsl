#version 460 core

// constants
const uint NB_BITS = 32; // k = 32
const uint NB_DIGIT = 4; // d = 4
const uint NB_DIGIT_PLACE = 8; // p = sup(k/d)

// inputs
const uint LOCAL_SIZE_X = (NB_DIGIT*NB_DIGIT_PLACE)>>1;
layout(local_size_x = LOCAL_SIZE_X) in;

layout (binding = 2, std430) buffer uGlobalHistogramSSBO {
    uint uGlobalHistogram[];
};

// outputs
layout (binding = 3, std430) buffer uGlobalHistogramScanSSBO {
    uint uGlobalHistogramScan[];
};

shared uint sLocalHistogramScan[LOCAL_SIZE_X*2];

// functions
void main(){
    uint id = gl_LocalInvocationIndex.x;

    // each invocations is responsible for the content of
    // two elements of the output array (for each digit places)
    for(uint i=0; i<NB_DIGIT_PLACE; i++){
        uint curId = id + (i*NB_DIGIT*2);
        sLocalHistogramScan[curId*2] = uGlobalHistogram[curId*2];
        sLocalHistogramScan[curId*2+1] = uGlobalHistogram[curId*2+1];
    }

    // sync to make sure that everyone has initialized 
    // their elements of shared data with data loaded from 
    // the input
    barrier();
    memoryBarrierShared();

    const uint steps = uint(log2(gl_WorkGroupSize.x / NB_DIGIT_PLACE)) + 1;
    uint rd_id[NB_DIGIT_PLACE];
    uint wr_id[NB_DIGIT_PLACE];
    uint mask[NB_DIGIT_PLACE];
    for(uint i = 0; i<steps; i++){
        for(uint j=0; j<NB_DIGIT_PLACE; j++){
            uint curId = id + (j*NB_DIGIT*2);
            // get read and write index
            mask[j] = (1 << i) - 1;
            rd_id[j] = ((curId >> i) << (i + 1)) + mask[j];
            wr_id[j] = rd_id[j] + 1 + (curId & mask[j]);

            // accumulate the read data into our element
            sLocalHistogramScan[wr_id[j]] += sLocalHistogramScan[rd_id[j]];
            
            // sync 
            barrier();
            memoryBarrierShared();
        }
    }

    // write data back to the output
    for(uint i=0; i<NB_DIGIT_PLACE; i++){
        uint curId = id + (i*NB_DIGIT*2);
        if((curId*2 + 1) % NB_DIGIT == 0){break;}
        uGlobalHistogramScan[curId*2 + 1] = sLocalHistogramScan[curId*2];
        if((curId*2 + 2) % NB_DIGIT == 0){break;}
        uGlobalHistogramScan[curId*2 + 2] = sLocalHistogramScan[curId*2+1];
    }
}