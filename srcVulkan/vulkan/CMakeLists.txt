set(VULKAN_SOURCE_FILES 
    vkCommands.cpp
    vkDescriptors.cpp
    vkDevice.cpp
    vkGraphicsQueue.cpp
    vkHelpers.cpp
    vkImageView.cpp
    vkInit.cpp
    vkInstance.cpp
    vkMemoryAllocator.cpp
    vkPhysicalDevice.cpp
    vkPipelines.cpp
    vkSlang.cpp
    vkSurface.cpp
    vkSwapChain.cpp
    vkSyncStructures.cpp
)

set(VULKAN_HEADER_FILES 
)

target_sources(commonVulkan 
    PUBLIC 
        ${VULKAN_HEADER_FILES}
    PRIVATE 
        ${VULKAN_SOURCE_FILES}
)

target_include_directories(commonVulkan 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)