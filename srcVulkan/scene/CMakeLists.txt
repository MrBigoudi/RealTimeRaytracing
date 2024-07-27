set(SCENE_SOURCE_FILES
    scene.cpp
)

set(SCENE_HEADER_FILES
    scene.hpp
)

target_sources(commonVulkan 
    PUBLIC 
        ${SCENE_HEADER_FILES}
    PRIVATE 
        ${SCENE_SOURCE_FILES}
)

target_include_directories(commonVulkan 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)