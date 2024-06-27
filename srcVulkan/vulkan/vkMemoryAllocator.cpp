#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "application.hpp"

namespace vkr{

void Application::initMemoryAllocator(){
    // initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _VulkanParameters._PhysicalDevice;
    allocatorInfo.device = _VulkanParameters._Device;
    allocatorInfo.instance = _VulkanParameters._Instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    VkResult result = vmaCreateAllocator(&allocatorInfo, &_VulkanParameters._Allocator);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to create the memory allocator!\n");
        exit(EXIT_FAILURE);
    }
}

void Application::destroyMemoryAllocator(){
    vmaDestroyAllocator(_VulkanParameters._Allocator);
}

}