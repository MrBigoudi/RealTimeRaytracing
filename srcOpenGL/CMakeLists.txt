# Add commonOpenGL library
add_library(commonOpenGL)

set(SOURCE_FILES 
    application.cpp
)

set(HEADER_FILES 
    application.hpp
)

target_sources(commonOpenGL 
    PUBLIC 
        ${HEADER_FILES}
    PRIVATE 
        ${SOURCE_FILES}
)

target_include_directories(commonOpenGL 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

add_subdirectory(dep)
add_subdirectory(scene)
add_subdirectory(shaders)

# Link library
target_link_libraries(commonOpenGL PRIVATE 
    common
)

# Add executable
add_executable(appVersionOpenGL main.cpp)

# Link dependencies
target_link_libraries(appVersionOpenGL PRIVATE 
    common
    commonOpenGL
    glfw 
    OpenGL
    OpenMP::OpenMP_CXX
    cflags 
    tinyobjloader
)