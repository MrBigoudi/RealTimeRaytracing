set(SCENE_SOURCE_FILES
    camera.cpp
)

set(SCENE_HEADER_FILES
    camera.hpp
)

target_sources(common 
    PUBLIC 
        ${SCENE_HEADER_FILES}
    PRIVATE 
        ${SCENE_SOURCE_FILES}
)

target_include_directories(common 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

add_subdirectory(geometry)
add_subdirectory(pbr)