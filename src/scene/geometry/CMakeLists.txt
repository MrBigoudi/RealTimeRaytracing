set(SCENE_GEOMETRY_SOURCE_FILES
    bvh.cpp
    mesh.cpp
    triangle.cpp
)

set(SCENE_GEOMETRY_HEADER_FILES
    bvh.hpp
    mesh.hpp
    triangle.hpp
)

target_sources(common 
    PUBLIC 
        ${SCENE_GEOMETRY_HEADER_FILES}
    PRIVATE 
        ${SCENE_GEOMETRY_SOURCE_FILES}
)

target_include_directories(common 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)