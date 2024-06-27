set(PIPELINES_SOURCE_FILES
    vkPipelines.cpp
    gradientPipeline.cpp
)

set(PIPELINES_HEADER_FILES 
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