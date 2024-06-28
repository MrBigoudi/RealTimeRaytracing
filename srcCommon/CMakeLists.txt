# Add common library
add_library(common)

set(SOURCE_FILES 
)

set(HEADER_FILES 
)

target_sources(common
    PUBLIC 
        ${HEADER_FILES}
    PRIVATE 
        ${SOURCE_FILES}
)

target_include_directories(common 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

add_subdirectory(core)
add_subdirectory(scene)

# Link the cflags library to the common library
target_link_libraries(common PRIVATE 
    glfw 
    OpenMP::OpenMP_CXX
    cflags 
    tinyobjloader
)