set(SCENE_PBR_SOURCE_FILES
    material.cpp
)

set(SCENE_PBR_HEADER_FILES
    material.hpp
)

target_sources(ntm 
    PUBLIC 
        ${SCENE_PBR_HEADER_FILES}
    PRIVATE
        ${SCENE_PBR_SOURCE_FILES}
)

target_include_directories(ntm 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)