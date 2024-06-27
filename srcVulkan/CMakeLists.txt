# Add commonVulkan library
add_library(commonVulkan)

add_subdirectory(dep)

set(SOURCE_FILES 
    glfwInit.cpp
    vulkanInit.cpp
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

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

# Link dependencies
target_link_libraries(appVersionVulkan PRIVATE 
    commonVulkan
    slang
    vk-bootstrap::vk-bootstrap
    glfw 
    Vulkan::Vulkan
    OpenMP::OpenMP_CXX
    cflags 
    tinyobjloader
    GPUOpen::VulkanMemoryAllocator
)