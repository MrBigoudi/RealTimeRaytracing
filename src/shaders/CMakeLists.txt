set(SHADERS_SOURCE_FILES
    program.cpp
    shader.cpp
)

set(SHADERS_HEADER_FILES
    program.hpp
    shader.hpp
)

target_sources(common 
    PUBLIC 
        ${SHADERS_HEADER_FILES}
    PRIVATE 
        ${SHADERS_SOURCE_FILES}
)

target_include_directories(common 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)