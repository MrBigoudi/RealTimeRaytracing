#include "application.hpp"

#include "errorHandler.hpp"

namespace vkr{

void Application::initSyncStructures(){
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = this->fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = this->semaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; i++) {
        VkResult result = vkCreateFence(
            _VulkanParameters._Device, 
            &fenceCreateInfo, 
            nullptr, 
            &_Frames[i]._RenderFence
        );
        cr::ErrorHandler::vulkanError(
            result == VK_SUCCESS,
            __FILE__, __LINE__,
            "Failed to create the fence!\n"
        ); 

		result = vkCreateSemaphore(
            _VulkanParameters._Device, 
            &semaphoreCreateInfo, 
            nullptr, 
            &_Frames[i]._SwapchainSemaphore
        );
        cr::ErrorHandler::vulkanError(
            result == VK_SUCCESS,
            __FILE__, __LINE__,
            "Failed to create the swapchain semaphore!\n"
        ); 

        result = vkCreateSemaphore(
            _VulkanParameters._Device, 
            &semaphoreCreateInfo, 
            nullptr, 
            &_Frames[i]._RenderSemaphore
        );
        cr::ErrorHandler::vulkanError(
            result == VK_SUCCESS,
            __FILE__, __LINE__,
            "Failed to create the render semaphore!\n"
        );       
	}
}

void Application::destroySyncStructures(){
    for (int i = 0; i < FRAME_OVERLAP; i++) {
		//destroy sync objects
		vkDestroyFence(
            _VulkanParameters._Device, 
            _Frames[i]._RenderFence, 
            nullptr
        );
		vkDestroySemaphore(
            _VulkanParameters._Device, 
            _Frames[i]._RenderSemaphore, nullptr
        );
		vkDestroySemaphore(
            _VulkanParameters._Device ,
            _Frames[i]._SwapchainSemaphore, nullptr
        );
	}
}

}


namespace vkr {

VkFenceCreateInfo Application::fenceCreateInfo(VkFenceCreateFlags flags){
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    return info;
}

VkSemaphoreCreateInfo Application::semaphoreCreateInfo(VkSemaphoreCreateFlags flags){
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    return info;
}

}