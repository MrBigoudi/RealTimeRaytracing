#include <iostream>
#include <cassert>
#include <random>

#include "application.hpp"

namespace glr{

///// constants
const uint32_t INPUT_BUFFER_SIZE = 32;
const uint32_t NB_DIGIT = 4; // d = 4
const uint32_t NB_DIGIT_PLACE = 8; // p = sup(k/d)
const uint32_t OUTPUT_BUFFER_SIZE = NB_DIGIT_PLACE*NB_DIGIT;


///// helpers
void initRandomValuesToSort(uint32_t* array, uint32_t nbValues, uint32_t minValue = 0, uint32_t maxValue = 8192){
    // Define a random number generator engine
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    // Define a distribution (for uint32_t values from 0 to UINT32_MAX)
    // std::uniform_int_distribution<std::uint32_t> distrib(0, std::numeric_limits<std::uint32_t>::max());
    std::uniform_int_distribution<std::uint32_t> distrib(minValue, maxValue);
    for(size_t i=0; i<nbValues; i++){
        array[i] = distrib(gen);
    }
}

void initRandomValuesHistogramInput(uint32_t* inputValuesToSort, uint32_t* inputArray){
    for(size_t i=0; i<INPUT_BUFFER_SIZE; i++){
        uint32_t value = inputValuesToSort[i];
        for(size_t p = 0; p<NB_DIGIT_PLACE; p++){
            for(size_t d = 0; d<NB_DIGIT; d++){
                uint32_t bitPosition = p*NB_DIGIT + d;
                uint32_t mask = (1 << bitPosition);
                if((value & mask) != 0)
                inputArray[bitPosition] ++;
            }
        }
    }
}

void initRandomValuesHistogramOutput(uint32_t* outputArray, uint32_t* inputArray){
    for(uint32_t j=0; j<NB_DIGIT_PLACE; j++){
        for(uint32_t i=0; i<NB_DIGIT; i++){
            if(i%NB_DIGIT == 0){continue;}
            uint32_t curIndex = j*NB_DIGIT + i;
            outputArray[curIndex] = inputArray[curIndex-1] + outputArray[curIndex-1];
        }
    }
}

void initKnownValuesHistogramInput(uint32_t* inputArray){
    inputArray[NB_DIGIT-1] = 53;   
    inputArray[NB_DIGIT-2] = 49;
    inputArray[NB_DIGIT-3] = 41;   
    inputArray[NB_DIGIT-4] = 37;  

    inputArray[2*NB_DIGIT-1] = 51;   
    inputArray[2*NB_DIGIT-2] = 44;
    inputArray[2*NB_DIGIT-3] = 48;   
    inputArray[2*NB_DIGIT-4] = 37;  

    inputArray[3*NB_DIGIT-1] = 41;   
    inputArray[3*NB_DIGIT-2] = 53;
    inputArray[3*NB_DIGIT-3] = 51;   
    inputArray[3*NB_DIGIT-4] = 35;  
}

void initKnownValuesHistogramOutput(uint32_t* outputArray){
    outputArray[NB_DIGIT-1] = 127;   
    outputArray[NB_DIGIT-2] = 78;
    outputArray[NB_DIGIT-3] = 37;   
    outputArray[NB_DIGIT-4] = 0;  

    outputArray[2*NB_DIGIT-1] = 129;   
    outputArray[2*NB_DIGIT-2] = 85;
    outputArray[2*NB_DIGIT-3] = 37;   
    outputArray[2*NB_DIGIT-4] = 0;  

    outputArray[3*NB_DIGIT-1] = 139;   
    outputArray[3*NB_DIGIT-2] = 86;
    outputArray[3*NB_DIGIT-3] = 35;   
    outputArray[3*NB_DIGIT-4] = 0;  
}

GLuint initBuffer(size_t bufferSize, uint32_t* buffer){
    GLuint ssbo;
    glCreateBuffers(1, &ssbo);
    assert(ssbo != 0);
    glNamedBufferStorage(ssbo, 
                    bufferSize, 
                    buffer, 
                    GL_DYNAMIC_STORAGE_BIT
    );
    return ssbo;
}

void sendBufferToGPU(GLuint ssbo, GLuint bufferBinding){
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferBinding, ssbo);
}

ProgramPtr loadComputeShader(){
    ShaderPtr computeShader = ShaderPtr(new Shader(
        Shader::SHADER_DIRECTORY 
        + "ploc/preprocessing/sort/prefixSumOfGlobalDigitCounts.glsl", 
        COMPUTE_SHADER));
    ProgramPtr program = ProgramPtr(new Program(computeShader));
    return program;
}

void displayBuffer(const std::string& name, uint32_t* buffer, size_t bufferSize){
    fprintf(stderr, "%s:\n[", name.c_str());
        for (uint32_t i = 0; i < bufferSize-1; i++) {
            fprintf(stderr, "%u, ", buffer[i]);
        }
        fprintf(stderr, "%u]\n", buffer[bufferSize-1]);
}

void runTest(ProgramPtr program, uint32_t* valuesToSort, uint32_t* expectedResults){
    // create input buffer
    size_t inputBufferSize = sizeof(uint32_t) * OUTPUT_BUFFER_SIZE;
    GLuint valuesToSortSSBO = initBuffer(inputBufferSize, valuesToSort);
    // create output buffer
    uint32_t globalHistogram[OUTPUT_BUFFER_SIZE] = {0};
    size_t outputBufferSize = sizeof(uint32_t) * OUTPUT_BUFFER_SIZE;
    GLuint globalHistogramSSBO = initBuffer(outputBufferSize, globalHistogram);
    // send input buffer
    program->use();
    GLuint inputBufferBinding = 2;
    sendBufferToGPU(valuesToSortSSBO, inputBufferBinding);
    // send output buffer
    GLuint outputBufferBinding = 3;
    sendBufferToGPU(globalHistogramSSBO, outputBufferBinding);
    // run the compute shader
    uint32_t nbWorkGroups = OUTPUT_BUFFER_SIZE;
    // uint32_t nbWorkGroups = NB_DIGIT;
    glDispatchCompute(nbWorkGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // retrieve the data from the output buffer
    glGetNamedBufferSubData(globalHistogramSSBO, 0, outputBufferSize, globalHistogram);
    
    // display input
    displayBuffer("input", valuesToSort, OUTPUT_BUFFER_SIZE);
    // display expected
    displayBuffer("expected", expectedResults, OUTPUT_BUFFER_SIZE);
    // display results
    displayBuffer("results", globalHistogram, OUTPUT_BUFFER_SIZE);

    // check values
    for(size_t i=0; i<OUTPUT_BUFFER_SIZE; i++){
        assert(globalHistogram[i] == expectedResults[i]);
    }
}


///// tests
void testRandomValues(ProgramPtr program){
    fprintf(stderr, "\nBegin test: random...\n");
    uint32_t valuesToSort[INPUT_BUFFER_SIZE] = {0};
    initRandomValuesToSort(valuesToSort, INPUT_BUFFER_SIZE);
    uint32_t input[OUTPUT_BUFFER_SIZE] = {0};
    initRandomValuesHistogramInput(valuesToSort, input);
    uint32_t expectedResults[OUTPUT_BUFFER_SIZE] = {0};
    initRandomValuesHistogramOutput(expectedResults, input);
    runTest(program, input, expectedResults);
    fprintf(stderr, "\tOk\n");
}

void testKnownValues(ProgramPtr program){
    fprintf(stderr, "\nBegin test: known values...\n");
    uint32_t valuesToSort[INPUT_BUFFER_SIZE] = {0};
    initKnownValuesHistogramInput(valuesToSort);
    uint32_t expectedResults[OUTPUT_BUFFER_SIZE] = {0};
    initKnownValuesHistogramOutput(expectedResults);
    runTest(program, valuesToSort, expectedResults);
    fprintf(stderr, "\tOk\n");
}
}

using namespace glr;


///// main
int main() {   
    Application app = Application::dummyApplication();
    ProgramPtr program = loadComputeShader();

    testKnownValues(program);
    testRandomValues(program);

    exit(EXIT_SUCCESS);
}