set(CMAKE_CTEST_OUTPUT_ON_FAILURE ON)

# Function to add a test executable and register it as a test
function(add_project_test TEST_NAME TEST_SOURCE)
    # Extract the directory structure from the test source
    get_filename_component(TEST_DIR ${TEST_SOURCE} DIRECTORY)
    
    # Initialize labels with "custom" and "tests"
    set(LABELS "customTests")
    
    if(TEST_DIR)
        # Replace "/" with ";" to create directory labels
        string(REPLACE "/" ";" DIR_LABELS ${TEST_DIR})
        
        # Add directory labels
        foreach(DIR ${DIR_LABELS})
            string(APPEND LABELS ";${DIR}")
        endforeach()
    endif()
    
    # Add the executable
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    # Link libraries
    target_link_libraries(${TEST_NAME} PRIVATE 
        common
        commonOpenGL
        commonVulkan
        glfw 
        OpenGL
        OpenMP::OpenMP_CXX
        cflags 
        tinyobjloader
    )
    
    # Register the test and set properties
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES LABELS "${LABELS}")

    # Write the labels to a file
    file(APPEND "${CMAKE_BINARY_DIR}/test_labels.txt" "${LABELS}\n")
endfunction()

# Clear the test labels file
file(WRITE "${CMAKE_BINARY_DIR}/test_labels.txt" "")

# Add tests
add_project_test(basic basic.cpp)

# Tests GPU sorting
add_project_test(histogramCreation testsSortGPU/testHistogramCreation.cpp)
add_project_test(histogramCreationHard testsSortGPU/testHistogramCreationHard.cpp)
add_project_test(histogramPrefixSum testsSortGPU/testHistogramPrefixSum.cpp)