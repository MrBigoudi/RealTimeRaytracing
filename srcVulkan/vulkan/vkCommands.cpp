#include "application.hpp"

namespace vkr{

void Application::initCommands(){
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = commandPoolCreateInfo(
        _VulkanParameters._GraphicsQueueFamilyIndex
    );
	
	for (int i = 0; i < FRAME_OVERLAP; i++) {
        VkResult result = vkCreateCommandPool(
            _VulkanParameters._Device, 
            &commandPoolInfo, 
            nullptr, 
            &_Frames[i]._CommandPool
        );
        if(result != VK_SUCCESS){
            fprintf(
                stderr,
                "Failed to create the command pools!\n"
            );
            exit(EXIT_FAILURE);
        }

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = commandBufferAllocateInfo(
            _Frames[i]._CommandPool
        );

		result = vkAllocateCommandBuffers(
            _VulkanParameters._Device, 
            &cmdAllocInfo, 
            &_Frames[i]._MainCommandBuffer
        );
        if(result != VK_SUCCESS){
            fprintf(
                stderr,
                "Failed to allocate the command buffers!\n"
            );
            exit(EXIT_FAILURE);
        }
	}
}

void Application::destroyCommands(){
    for (int i = 0; i < FRAME_OVERLAP; i++) {
        vkDestroyCommandPool(
            _VulkanParameters._Device, 
            _Frames[i]._CommandPool, 
            nullptr
        );
    }
}

VkCommandPoolCreateInfo Application::commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags){
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = flags;
    return info;
}


VkCommandBufferAllocateInfo Application::commandBufferAllocateInfo(VkCommandPool pool, uint32_t count){
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandPool = pool;
    info.commandBufferCount = count;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    return info;
}

}