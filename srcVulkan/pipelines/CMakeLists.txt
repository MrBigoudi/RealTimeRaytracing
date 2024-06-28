set(PIPELINES_SOURCE_FILES
    gradientPipeline.cpp
    pipeline.cpp
)

set(PIPELINES_HEADER_FILES 
    gradientPipeline.hpp
)

target_sources(commonVulkan 
    PUBLIC 
        ${PIPELINES_HEADER_FILES}
    PRIVATE 
        ${PIPELINES_SOURCE_FILES}
)

target_include_directories(commonVulkan 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)