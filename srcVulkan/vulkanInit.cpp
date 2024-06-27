#include "dep/slang/source/core/slang-list.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "application.hpp"

#include <fstream>

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
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(_Window, &width, &height);

    // setup builder
    vkb::SwapchainBuilder swapchain_builder{_VulkanParameters._Device};
    swapchain_builder
        //.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height))
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

VkImageCreateInfo Application::imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent){
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
    info.samples = VK_SAMPLE_COUNT_1_BIT;

    //optimal tiling, which means the image is stored on the best gpu format
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

VkImageViewCreateInfo Application::imageviewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags){
    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}

void Application::createImage(VkImageCreateInfo* rimg_info, VmaAllocationCreateInfo* rimg_allocinfo){
	VkResult result = vmaCreateImage(_VulkanParameters._Allocator, rimg_info, rimg_allocinfo, &_DrawImage._Image, &_DrawImage._Allocation, nullptr);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to create an image!\n");
        exit(EXIT_FAILURE);
    }    
}

void Application::createImageView(VkImageViewCreateInfo* rview_info){
	VkResult result = vkCreateImageView(
        _VulkanParameters._Device, 
        rview_info, 
        nullptr, 
        &_DrawImage._ImageView
    );
}

void Application::initImageView(){
    //draw image size will match the window
	VkExtent3D drawImageExtent = {
		_Parameters._WindowWidth,
		_Parameters._WindowHeight,
		1
	};

	//hardcoding the draw format to 32 bit float
	_DrawImage._ImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	_DrawImage._ImageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = imageCreateInfo(_DrawImage._ImageFormat, drawImageUsages, drawImageExtent);

	//for the draw image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	createImage(&rimg_info, &rimg_allocinfo);

	//build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo rview_info = imageviewCreateInfo(_DrawImage._ImageFormat, _DrawImage._Image, VK_IMAGE_ASPECT_COLOR_BIT);
    createImageView(&rview_info);
}

void Application::destroyImageView(){
    vkDestroyImageView(_VulkanParameters._Device, _DrawImage._ImageView, nullptr);
    vmaDestroyImage(_VulkanParameters._Allocator, _DrawImage._Image, _DrawImage._Allocation);
}

void Application::copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize){
	VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

	blitRegion.srcOffsets[1].x = srcSize.width;
	blitRegion.srcOffsets[1].y = srcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dstSize.width;
	blitRegion.dstOffsets[1].y = dstSize.height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
	blitInfo.dstImage = destination;
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = source;
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(cmd, &blitInfo);
}

void DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type){
    VkDescriptorSetLayoutBinding newbind {};
    newbind.binding = binding;
    newbind.descriptorCount = 1;
    newbind.descriptorType = type;
    _Bindings.push_back(newbind);
}

void DescriptorLayoutBuilder::clear(){
    _Bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
{
    for (VkDescriptorSetLayoutBinding& b : _Bindings) {
        b.stageFlags |= shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.pNext = pNext;

    info.pBindings = _Bindings.data();
    info.bindingCount = static_cast<uint32_t>(_Bindings.size());
    info.flags = flags;

    VkDescriptorSetLayout set;
    VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &set);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to create the descriptor set layout!\n");
        exit(EXIT_FAILURE);
    }

    return set;
}

void DescriptorAllocator::initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios){
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio._Type,
            .descriptorCount = uint32_t(ratio._Ratio * maxSets)
        });
    }

	VkDescriptorPoolCreateInfo pool_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	pool_info.flags = 0;
	pool_info.maxSets = maxSets;
	pool_info.poolSizeCount = (uint32_t)poolSizes.size();
	pool_info.pPoolSizes = poolSizes.data();

	VkResult result = vkCreateDescriptorPool(device, &pool_info, nullptr, &_Pool);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to allocate the descriptor pool!\n");
        exit(EXIT_FAILURE);
    }
}

void DescriptorAllocator::clearDescriptors(VkDevice device){
    VkResult result = vkResetDescriptorPool(device, _Pool, 0);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to reset the pool!\n");
        exit(EXIT_FAILURE);
    }
}

void DescriptorAllocator::destroyPool(VkDevice device){
    vkDestroyDescriptorPool(device,_Pool,nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = _Pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds;
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &ds);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to allocate the descriptor sets!\n");
        exit(EXIT_FAILURE);
    }

    return ds;
}

void Application::initDescriptors(){
	//create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes ={
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
	};

	_GlobalDescriptorAllocator.initPool(_VulkanParameters._Device, 10, sizes);

	//make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		_DrawImageDescriptorLayout = builder.build(_VulkanParameters._Device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

    //allocate a descriptor set for our draw image
	_DrawImageDescriptors = _GlobalDescriptorAllocator.allocate(_VulkanParameters._Device, _DrawImageDescriptorLayout);	

	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = _DrawImage._ImageView;
	
	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;
	
	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = _DrawImageDescriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(_VulkanParameters._Device, 1, &drawImageWrite, 0, nullptr);
}

void Application::destroyDescriptors(){
    _GlobalDescriptorAllocator.destroyPool(_VulkanParameters._Device);
    vkDestroyDescriptorSetLayout(_VulkanParameters._Device, _DrawImageDescriptorLayout, nullptr);
}

bool Application::loadShaderModule(Slang::ComPtr<slang::IBlob> spirvCode, VkDevice device, VkShaderModule* outShaderModule){

    // create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    // codeSize has to be in bytes, so multply the ints in the buffer by size of
    // int to know the real size of the buffer
    createInfo.codeSize = spirvCode->getBufferSize();
    createInfo.pCode = static_cast<const uint32_t*>(spirvCode->getBufferPointer());

    // check that the creation goes well.
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }
    *outShaderModule = shaderModule;
    return true;
}

void Application::initPipelines(){
    initBackgroundPipelines();
}

void Application::destroyPipelines(){
    destroyBackgroundPipelines();
}

void Application::initBackgroundPipelines(){
    const std::string COMPILED_SHADER_DIRECTORY = std::string(PROJECT_SOURCE_DIR) + "/shaders/compiled/";
    // Once the slang session has been obtained, we can start loading code into it.
    //
    // The simplest way to load code is by calling `loadModule` with the name of a Slang
    // module. A call to `loadModule("hello-world")` will behave more or less as if you
    // wrote:
    //
    //      import hello_world;
    //
    // In a Slang shader file. The compiler will use its search paths to try to locate
    // `hello-world.slang`, then compile and load that file. If a matching module had
    // already been loaded previously, that would be used directly.
    slang::IModule* slangModule = nullptr;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        slangModule = _Slang._Session->loadModule((COMPILED_SHADER_DIRECTORY + "gradient").c_str(), diagnosticsBlob.writeRef());
        if(!slangModule){
            fprintf(stderr, 
            "Failed to load the slang module: %s\n",
            (const char*)diagnosticsBlob->getBufferPointer());
            exit(EXIT_FAILURE);
        }
    }

    // Loading the `hello-world` module will compile and check all the shader code in it,
    // including the shader entry points we want to use. Now that the module is loaded
    // we can look up those entry points by name.
    //
    // Note: If you are using this `loadModule` approach to load your shader code it is
    // important to tag your entry point functions with the `[shader("...")]` attribute
    // (e.g., `[shader("compute")] void computeMain(...)`). Without that information there
    // is no umambiguous way for the compiler to know which functions represent entry
    // points when it parses your code via `loadModule()`.
    //
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    slangModule->findEntryPointByName("computeMain", entryPoint.writeRef());

    // At this point we have a few different Slang API objects that represent
    // pieces of our code: `module`, `vertexEntryPoint`, and `fragmentEntryPoint`.
    //
    // A single Slang module could contain many different entry points (e.g.,
    // four vertex entry points, three fragment entry points, and two compute
    // shaders), and before we try to generate output code for our target API
    // we need to identify which entry points we plan to use together.
    //
    // Modules and entry points are both examples of *component types* in the
    // Slang API. The API also provides a way to build a *composite* out of
    // other pieces, and that is what we are going to do with our module
    // and entry points.
    //
    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(slangModule);
    componentTypes.add(entryPoint);

    // Actually creating the composite component type is a single operation
    // on the Slang session, but the operation could potentially fail if
    // something about the composite was invalid (e.g., you are trying to
    // combine multiple copies of the same module), so we need to deal
    // with the possibility of diagnostic output.
    //
    Slang::ComPtr<slang::IComponentType> composedProgram;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = _Slang._Session->createCompositeComponentType(
            componentTypes.getBuffer(),
            componentTypes.getCount(),
            composedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        if(result != 0){
            fprintf(stderr, 
            "Failed to compose the slang program: %s\n",
            (const char*)diagnosticsBlob->getBufferPointer());
            exit(EXIT_FAILURE);
        }
    }

    // Now we can call `composedProgram->getEntryPointCode()` to retrieve the
    // compiled SPIRV code that we will use to create a vulkan compute pipeline.
    // This will trigger the final Slang compilation and spirv code generation.
    Slang::ComPtr<slang::IBlob> spirvCode;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = composedProgram->getEntryPointCode(
            0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
        if(result != 0){
            fprintf(stderr, 
            "Failed to get the slang compiled program: %s\n",
            (const char*)diagnosticsBlob->getBufferPointer());
            exit(EXIT_FAILURE);
        }
    }


    VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &_DrawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

	VkResult result = vkCreatePipelineLayout(_VulkanParameters._Device, &computeLayout, nullptr, &_gradientPipelineLayout);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to init the backgroud pipelines layouts!\n");
        exit(EXIT_FAILURE);
    }

    //layout code
	VkShaderModule computeDrawShader;
	if (!loadShaderModule(spirvCode, _VulkanParameters._Device, &computeDrawShader)){
		fprintf(stderr, "Error when building the compute shader!\n");
        exit(EXIT_FAILURE);
	}

	VkPipelineShaderStageCreateInfo stageinfo{};
	stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageinfo.pNext = nullptr;
	stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageinfo.module = computeDrawShader;
	stageinfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = _gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;
	
	result = vkCreateComputePipelines(_VulkanParameters._Device,VK_NULL_HANDLE,1,&computePipelineCreateInfo, nullptr, &_gradientPipeline);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to create the compute shader pipelines!\n");
        exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(_VulkanParameters._Device, computeDrawShader, nullptr);
}

void Application::destroyBackgroundPipelines(){
    vkDestroyPipelineLayout(_VulkanParameters._Device, _gradientPipelineLayout, nullptr);
    vkDestroyPipeline(_VulkanParameters._Device, _gradientPipeline, nullptr);
}

void Application::initSlang(){
    // First we need to create slang global session with work with the Slang API.
    SlangResult result = slang::createGlobalSession(_Slang._GlobalSession.writeRef());
    if(result != 0){
        fprintf(stderr, "Failed to create the slang global session!\n");
        exit(EXIT_FAILURE);
    }

    // Next we create a compilation session to generate SPIRV code from Slang source.
    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = _Slang._GlobalSession->findProfile("spirv_1_5");
    targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    // Create session
    result = _Slang._GlobalSession->createSession(sessionDesc, _Slang._Session.writeRef());
    if(result != 0){
        fprintf(stderr, "Failed to create the slang session!\n");
        exit(EXIT_FAILURE);
    }
}

void Application::destroySlang(){}


}

namespace vkr {

void Application::initVulkanParameters(){
    initInstance();
    initSurface();
    initPhysicalDevice();
    initDevice();
    initGraphicsQueue();
    initMemoryAllocator();
    initSwapChain();
    initImageView();
    initCommands();
    initSyncStructures();
    initDescriptors();
    initSlang();
    initPipelines();
}

void Application::destroyVulkanParameters(){
    destroyPipelines();
    destroySlang();
    destroyDescriptors();
    destroySyncStructures();
    destroyCommands();
    destroyImageView();
    destroySwapChain();
    destroyMemoryAllocator();
    destroyGraphicsQueue();
    destroyDevice();
    destroyPhysicalDevice();
    destroySurface();
    destroyInstance();
}

}