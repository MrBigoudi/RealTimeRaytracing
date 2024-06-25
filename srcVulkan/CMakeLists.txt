# Add commonVulkan library
add_library(commonVulkan)

set(SOURCE_FILES 
    application.cpp
)

set(HEADER_FILES 
    application.hpp
)

target_sources(commonVulkan 
    PUBLIC 
        ${HEADER_FILES}
    PRIVATE 
        ${SOURCE_FILES}
)

target_include_directories(commonVulkan 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

# Add executable
add_executable(appVersionVulkan main.cpp)

# Link dependencies
target_link_libraries(appVersionVulkan PRIVATE 
    commonVulkan
    glfw 
    Vulkan::Vulkan 
    OpenMP::OpenMP_CXX
    cflags 
    tinyobjloader
)