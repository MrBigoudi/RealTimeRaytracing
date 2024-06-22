# Function to add a test executable and register it as a test
function(add_project_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE 
        ntm
        glfw 
        OpenGL
        OpenMP::OpenMP_CXX
        cflags 
        tinyobjloader
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Add the sorting test
add_project_test(SortTest testSort.cpp)