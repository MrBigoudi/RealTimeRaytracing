#include <iostream>
#include <cassert>
#include <random>

#include "application.hpp"

namespace glr{

///// constants
const uint32_t INPUT_BUFFER_SIZE = 130;
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

void initRandomValuesExpectedResults(uint32_t* inputArray, uint32_t* outputArray){
    for(size_t i=0; i<INPUT_BUFFER_SIZE; i++){
        uint32_t value = inputArray[i];
        for(size_t p = 0; p<NB_DIGIT_PLACE; p++){
            for(size_t d = 0; d<NB_DIGIT; d++){
                uint32_t bitPosition = p*NB_DIGIT + d;
                uint32_t mask = (1 << bitPosition);
                if((value & mask) != 0)
                outputArray[bitPosition] ++;
            }
        }
    }
}

void initPowersOfTwoValuesToSort(uint32_t* inputArray, uint32_t nbValues){
    for(size_t i=0; i<nbValues; i++){
        inputArray[i] = (1 << (i % 32));
    }
}

void initPowersOfTwoExpectedResults(uint32_t* outputArray, uint32_t nbValues){
    for(size_t i=0; i<INPUT_BUFFER_SIZE; i++){
        outputArray[i%nbValues]++;
    }
}

void initOnesToSort(uint32_t* inputArray, uint32_t nbValues){
    for(size_t i=0; i<nbValues; i++){
        inputArray[i] = 1;
    }
}

void initTwosToSort(uint32_t* inputArray, uint32_t nbValues){
    for(size_t i=0; i<nbValues; i++){
        inputArray[i] = 2;
    }
}

void initOnesExpectedResults(uint32_t* outputArray){
    outputArray[0] = INPUT_BUFFER_SIZE;
}

void initTwosExpectedResults(uint32_t* outputArray){
    outputArray[1] = INPUT_BUFFER_SIZE;
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
        + "ploc/preprocessing/sort/histogramOfGlobalDigitCounts.glsl", 
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
    size_t inputBufferSize = sizeof(uint32_t) * INPUT_BUFFER_SIZE;
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
    // send uniforms
    program->setUInt("uNbValuesToSort", INPUT_BUFFER_SIZE);
    // run the compute shader
    uint32_t nbWorkGroups = std::ceil(std::max(static_cast<uint32_t>(1),INPUT_BUFFER_SIZE / (16*16*8)));
    glDispatchCompute(nbWorkGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // retrieve the data from the output buffer
    glGetNamedBufferSubData(globalHistogramSSBO, 0, outputBufferSize, globalHistogram);
    
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
void testPowersOfTwo(ProgramPtr program){
    fprintf(stderr, "\nBegin test: powers of two...\n");
    uint32_t valuesToSort[INPUT_BUFFER_SIZE] = {0};
    initPowersOfTwoValuesToSort(valuesToSort, INPUT_BUFFER_SIZE);
    uint32_t expectedResults[OUTPUT_BUFFER_SIZE] = {0};
    initPowersOfTwoExpectedResults(expectedResults, OUTPUT_BUFFER_SIZE);
    runTest(program, valuesToSort, expectedResults);
    fprintf(stderr, "\tOk\n");
}

void testZeros(ProgramPtr program){
    fprintf(stderr, "\nBegin test: zeros...\n");
    uint32_t valuesToSort[INPUT_BUFFER_SIZE] = {0};
    uint32_t expectedResults[OUTPUT_BUFFER_SIZE] = {0};
    runTest(program, valuesToSort, expectedResults);
    fprintf(stderr, "\tOk\n");
}

void testOnes(ProgramPtr program){
    fprintf(stderr, "\nBegin test: ones...\n");
    uint32_t valuesToSort[INPUT_BUFFER_SIZE] = {0};
    initOnesToSort(valuesToSort, INPUT_BUFFER_SIZE);
    uint32_t expectedResults[OUTPUT_BUFFER_SIZE] = {0};
    initOnesExpectedResults(expectedResults);
    runTest(program, valuesToSort, expectedResults);
    fprintf(stderr, "\tOk\n");
}

void testTwos(ProgramPtr program){
    fprintf(stderr, "\nBegin test: twos...\n");
    uint32_t valuesToSort[INPUT_BUFFER_SIZE] = {0};
    initTwosToSort(valuesToSort, INPUT_BUFFER_SIZE);
    uint32_t expectedResults[OUTPUT_BUFFER_SIZE] = {0};
    initTwosExpectedResults(expectedResults);
    runTest(program, valuesToSort, expectedResults);
    fprintf(stderr, "\tOk\n");
}

void testRandomValues(ProgramPtr program){
    fprintf(stderr, "\nBegin test: random...\n");
    uint32_t valuesToSort[INPUT_BUFFER_SIZE] = {0};
    initRandomValuesToSort(valuesToSort, INPUT_BUFFER_SIZE);
    uint32_t expectedResults[OUTPUT_BUFFER_SIZE] = {0};
    initRandomValuesExpectedResults(valuesToSort, expectedResults);
    runTest(program, valuesToSort, expectedResults);
    fprintf(stderr, "\tOk\n");
}

}

using namespace glr;

///// main
int main() {   
    Application app = Application::dummyApplication();
    ProgramPtr program = loadComputeShader();

    testPowersOfTwo(program);
    testZeros(program);
    testOnes(program);
    testTwos(program);
    testRandomValues(program);

    exit(EXIT_SUCCESS);
}
