# Add common library
add_library(ntm)

set(SOURCE_FILES 
    application.cpp
)

set(HEADER_FILES 
    application.hpp
)

target_sources(ntm 
    PUBLIC 
        ${HEADER_FILES}
    PRIVATE 
        ${SOURCE_FILES}
)

target_include_directories(ntm 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

add_subdirectory(dep)
add_subdirectory(core)
add_subdirectory(scene)
add_subdirectory(shaders)