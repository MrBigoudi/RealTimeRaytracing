set(CMAKE_CTEST_OUTPUT_ON_FAILURE ON)

# Function to add a test executable and register it as a test
function(add_project_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE 
        commonOpenGL
        glfw 
        OpenGL
        OpenMP::OpenMP_CXX
        cflags 
        tinyobjloader
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Add the tests

# Tests GPU sorting
add_project_test(histogramCreation testSortGPU/testHistogramCreation.cpp)
add_project_test(histogramCreationHard testSortGPU/testHistogramCreationHard.cpp)
add_project_test(histogramPrefixSum testSortGPU/testHistogramPrefixSum.cpp)