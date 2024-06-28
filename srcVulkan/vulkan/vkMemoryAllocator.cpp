#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "application.hpp"

#include "errorHandler.hpp"

namespace vkr{

void Application::initMemoryAllocator(){
    // initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _VulkanParameters._PhysicalDevice;
    allocatorInfo.device = _VulkanParameters._Device;
    allocatorInfo.instance = _VulkanParameters._Instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    VkResult result = vmaCreateAllocator(&allocatorInfo, &_VulkanParameters._Allocator);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to create the memory allocator!\n"
    ); 
}

void Application::destroyMemoryAllocator(){
    vmaDestroyAllocator(_VulkanParameters._Allocator);
}

}