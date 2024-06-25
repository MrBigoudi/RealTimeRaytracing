#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.hpp"

namespace vkr{

void Application::initInstance(){
    vkb::InstanceBuilder instance_builder;
    // configure the instance builder
    instance_builder
        .request_validation_layers()
        .use_default_debug_messenger()
        .set_app_name(_Parameters._WindowTitle.c_str())
        .require_api_version(_Parameters._VulkanVersionMajor,_Parameters._VulkanVersionMinor,_Parameters._VulkanVersionPatch);

    // configure extensions
    auto system_info_ret = vkb::SystemInfo::get_system_info();
    if (!system_info_ret) {
        fprintf(stderr, 
            "Failed to get the system info: %s\n", 
            system_info_ret.error().message().c_str()
        );
        exit(EXIT_FAILURE);
    }
    auto system_info = system_info_ret.value();
    if (system_info.is_layer_available("VK_LAYER_LUNARG_api_dump")) {
        instance_builder.enable_layer("VK_LAYER_LUNARG_api_dump");
    }
    if (system_info.validation_layers_available){
        instance_builder.enable_validation_layers();
    }

    // build the instance
    auto instance_builder_return = instance_builder.build();

    // check errors
    if (!instance_builder_return){
        fprintf(
            stderr, 
            "Failed to create Vulkan Instance: %s\n",
            instance_builder_return.error().message().c_str()
        );
    } 
    // get the instance
    _VulkanParameters._Instance = instance_builder_return.value();
}

void Application::destroyInstance(){
    vkb::destroy_instance(_VulkanParameters._Instance);
}

void Application::initSurface(){
    VkResult err = glfwCreateWindowSurface(_VulkanParameters._Instance, _Window, NULL, &_VulkanParameters._Surface);
    if (err != VK_SUCCESS) { 
        fprintf(
            stderr,
            "Failed to create the surface!\n"
        );
        exit(EXIT_FAILURE);
    }
}

void Application::destroySurface(){
    vkDestroySurfaceKHR(_VulkanParameters._Instance, _VulkanParameters._Surface, nullptr);
}

void Application::initPhysicalDevice(){
    vkb::PhysicalDeviceSelector phys_device_selector(_VulkanParameters._Instance); 
    auto physical_device_selector_return = phys_device_selector
        .set_surface(_VulkanParameters._Surface)
        // Add required extensions
        .add_required_extension("VK_KHR_timeline_semaphore")
        // Add wanted extensions
        // .add_desired_extension("VK_KHR_imageless_framebuffer")
        .select();
    if(!physical_device_selector_return) {
        fprintf(
            stderr,
            "Couldn't find a physical device!\n"
        );
        exit(EXIT_FAILURE);
    }
    _VulkanParameters._PhysicalDevice = physical_device_selector_return.value();
}

void Application::destroyPhysicalDevice(){
    // automatically cleaned
    // no need to destroy a physical device
}

void Application::initDevice(){
    vkb::DeviceBuilder device_builder{_VulkanParameters._PhysicalDevice};
    auto dev_ret = device_builder.build ();
    if (!dev_ret) {
        fprintf(
            stderr,
            "Failed to create the device!\n"
        );
        exit(EXIT_FAILURE);
    }
    _VulkanParameters._Device = dev_ret.value();
}

void Application::destroyDevice(){
    vkb::destroy_device(_VulkanParameters._Device);
}

void Application::initGraphicsQueue(){
    auto queue_ret = _VulkanParameters._Device.get_queue(vkb::QueueType::graphics);
    if (!queue_ret) {
        fprintf(
            stderr,
            "Failed to get the graphics queue!\n"
        );
        exit(EXIT_FAILURE);
    }
    auto queue_index_ret = _VulkanParameters._Device.get_queue_index(vkb::QueueType::graphics);
    if (!queue_index_ret) {
        fprintf(
            stderr,
            "Failed to get the graphics queue family index!\n"
        );
        exit(EXIT_FAILURE);
    }
    _VulkanParameters._GraphicsQueue = queue_ret.value();
    _VulkanParameters._GraphicsQueueFamilyIndex = queue_index_ret.value();
}

void Application::destroyGraphicsQueue(){
    // automatically cleaned
}

void Application::initSwapChain(bool recreate){
    // get width and height in pixels
    int width, height;
    glfwGetFramebufferSize(_Window, &width, &height);

    // setup builder
    vkb::SwapchainBuilder swapchain_builder{_VulkanParameters._Device};
    swapchain_builder
        //.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    // create or recreate swap chain
    auto swap_ret = (!_VulkanParameters._SwapChain 
        || _VulkanParameters._SwapChain.swapchain == VK_NULL_HANDLE
        || !recreate) 
            // init
            ? swapchain_builder.build()
            // recreate
            : swapchain_builder.set_old_swapchain(_VulkanParameters._SwapChain).build();

    // check errors
    if(!swap_ret){
        fprintf(
            stderr,
            "Failed to create a swapchain!\n"
        );
        _VulkanParameters._SwapChain.swapchain = VK_NULL_HANDLE;
    }
    _VulkanParameters._SwapChain = swap_ret.value();
}

void Application::destroySwapChain(){
    vkb::destroy_swapchain(_VulkanParameters._SwapChain);
}

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

VkCommandBufferBeginInfo Application::commandBufferBeginInfo(VkCommandBufferUsageFlags flags){
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

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

VkSemaphoreSubmitInfo Application::semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore){
	VkSemaphoreSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.semaphore = semaphore;
	submitInfo.stageMask = stageMask;
	submitInfo.deviceIndex = 0;
	submitInfo.value = 1;
	return submitInfo;
}

VkCommandBufferSubmitInfo Application::commandBufferSubmitInfo(VkCommandBuffer cmd){
	VkCommandBufferSubmitInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	info.pNext = nullptr;
	info.commandBuffer = cmd;
	info.deviceMask = 0;
	return info;
}

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
        if(result != VK_SUCCESS){
            fprintf(stderr, "Failed to create the fence!\n");
            exit(EXIT_FAILURE);
        }

		result = vkCreateSemaphore(
            _VulkanParameters._Device, 
            &semaphoreCreateInfo, 
            nullptr, 
            &_Frames[i]._SwapchainSemaphore
        );
        if(result != VK_SUCCESS){
            fprintf(stderr, "Failed to create the swapchain semaphore!\n");
            exit(EXIT_FAILURE);
        }

        result = vkCreateSemaphore(
            _VulkanParameters._Device, 
            &semaphoreCreateInfo, 
            nullptr, 
            &_Frames[i]._RenderSemaphore
        );
        if(result != VK_SUCCESS){
            fprintf(stderr, "Failed to create the render semaphore!\n");
            exit(EXIT_FAILURE);
        }        
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

FrameData& Application::getCurrentFrame(){
    return _Frames[_FrameNumber % FRAME_OVERLAP];
}

void Application::waitFences(uint32_t fenceCount, bool shouldWaitAll, uint32_t timeoutInNs){
    VkResult result = vkWaitForFences(
        _VulkanParameters._Device, 
        fenceCount, 
        &getCurrentFrame()._RenderFence, 
        shouldWaitAll, 
        timeoutInNs
    );
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to wait for fences!\n");
        exit(EXIT_FAILURE);
    }
}

void Application::resetFences(uint32_t fenceCount){
    VkResult result = vkResetFences(
        _VulkanParameters._Device, 
        fenceCount, 
        &getCurrentFrame()._RenderFence
    );
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to reset fences!\n");
        exit(EXIT_FAILURE);
    }
}

uint32_t Application::acquireNextImage(uint32_t timeoutInNs){
    uint32_t swapchainImageIndex;
    VkResult result = vkAcquireNextImageKHR(
        _VulkanParameters._Device, 
        _VulkanParameters._SwapChain, 
        timeoutInNs, 
        getCurrentFrame()._SwapchainSemaphore, 
        nullptr, 
        &swapchainImageIndex
    );
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to acquire next image!\n");
        exit(EXIT_FAILURE);
    }
    return swapchainImageIndex;
}

void Application::resetCommandBuffer(VkCommandBuffer commandBuffer){
    VkResult result = vkResetCommandBuffer(commandBuffer, 0);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to reset the command buffer!\n");
        exit(EXIT_FAILURE);
    }
}

void Application::endCommandBuffer(VkCommandBuffer commandBuffer){
    VkResult result = vkEndCommandBuffer(commandBuffer);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to end the command buffer!\n");
        exit(EXIT_FAILURE);
    }
}

void Application::beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferBeginInfo commandBufferBeginInfo){
    VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to begin the command buffer!\n");
        exit(EXIT_FAILURE);
    }
}

VkImageSubresourceRange Application::imageSubresourceRange(VkImageAspectFlags aspectMask){
    VkImageSubresourceRange subImage{};
    subImage.aspectMask = aspectMask;
    subImage.baseMipLevel = 0;
    subImage.levelCount = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;
    return subImage;
}

void Application::transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    imageBarrier.pNext = nullptr;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange = imageSubresourceRange(aspectMask);
    imageBarrier.image = image;

    VkDependencyInfo depInfo {};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

std::vector<VkImage> Application::getSwapChainImages(){
    auto swapchain_images_ret = _VulkanParameters._SwapChain.get_images();
    if(!swapchain_images_ret){
        fprintf(stderr, "Failed to fetch the swapchain images!\n");
        exit(EXIT_FAILURE);
    }
    return swapchain_images_ret.value();
}

VkSubmitInfo2 Application::submitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo){
    VkSubmitInfo2 info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info.pNext = nullptr;

    info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
    info.pWaitSemaphoreInfos = waitSemaphoreInfo;

    info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
    info.pSignalSemaphoreInfos = signalSemaphoreInfo;

    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos = cmd;

    return info;
}

void Application::queueSubmit2(VkSubmitInfo2* submit){
	VkResult result = vkQueueSubmit2(
        _VulkanParameters._GraphicsQueue, 
        1, 
        submit, 
        getCurrentFrame()._RenderFence
    );

    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to submit the queue!\n");
        exit(EXIT_FAILURE);
    }
}

VkPresentInfoKHR Application::getPresentInfo(uint32_t* swapchainImageIndex){
    VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &_VulkanParameters._SwapChain.swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &getCurrentFrame()._RenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = swapchainImageIndex;

    return presentInfo;
}

void Application::queuePresent(VkPresentInfoKHR* presentInfo){
	VkResult result = vkQueuePresentKHR(_VulkanParameters._GraphicsQueue, presentInfo);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to present the queue!\n");
        exit(EXIT_FAILURE);
    }
}


}

namespace vkr {

void Application::initVulkanParameters(){
    initInstance();
    initSurface();
    initPhysicalDevice();
    initDevice();
    initGraphicsQueue();
    initSwapChain();
    initCommands();
    initSyncStructures();
}

void Application::destroyVulkanParameters(){
    destroySyncStructures();
    destroyCommands();
    destroySwapChain();
    destroyGraphicsQueue();
    destroyDevice();
    destroyPhysicalDevice();
    destroySurface();
    destroyInstance();
}

}