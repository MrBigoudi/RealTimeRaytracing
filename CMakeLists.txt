cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0072 NEW)

project(nvidiaRaytracing)

# Enable testing
enable_testing()

# Set the path to the project directory
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR})

# Add the definition to the compiler flags
add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define compiler flags
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(common_flags "$<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>")
set(debug_flags "$<$<CONFIG:Debug>:-g>")
set(release_flags "$<$<CONFIG:Release>:-O2>")
set(cflags ${PROJECT_NAME}_compiler_flags)

# Create interface library for compiler flags
add_library(cflags INTERFACE)
target_compile_features(cflags INTERFACE cxx_std_20)
target_compile_options(cflags INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:${common_flags}>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
    "$<BUILD_INTERFACE:${debug_flags}>"
    "$<BUILD_INTERFACE:${release_flags}>"
)

# Find dependencies
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)
find_package(tinyobjloader REQUIRED)

# Add subdirectories
add_subdirectory(srcOpenGL)
add_subdirectory(srcVulkan)
add_subdirectory(tests)