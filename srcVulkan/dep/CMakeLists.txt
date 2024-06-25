add_subdirectory(vk-bootstrap)

target_sources(commonVulkan
    PUBLIC 
    PRIVATE 
)

target_include_directories(commonVulkan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vk-bootstrap/src
)