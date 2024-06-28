#include "application.hpp"

#include "errorHandler.hpp"

namespace vkr {

void Application::transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout){
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

VkImageSubresourceRange Application::imageSubresourceRange(VkImageAspectFlags aspectMask){
    VkImageSubresourceRange subImage{};
    subImage.aspectMask = aspectMask;
    subImage.baseMipLevel = 0;
    subImage.levelCount = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;
    return subImage;
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

VkCommandBufferBeginInfo Application::commandBufferBeginInfo(VkCommandBufferUsageFlags flags){
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
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
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to wait for fences!\n"
    );
}

void Application::resetFences(uint32_t fenceCount){
    VkResult result = vkResetFences(
        _VulkanParameters._Device, 
        fenceCount, 
        &getCurrentFrame()._RenderFence
    );
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to reset fences!\n"
    );
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
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to acquire next image!\n"
    );
    return swapchainImageIndex;
}

void Application::resetCommandBuffer(VkCommandBuffer commandBuffer){
    VkResult result = vkResetCommandBuffer(commandBuffer, 0);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to reset the command buffer!\n"
    );
}

void Application::endCommandBuffer(VkCommandBuffer commandBuffer){
    VkResult result = vkEndCommandBuffer(commandBuffer);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to end the command buffer!\n"
    );
}

void Application::beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferBeginInfo commandBufferBeginInfo){
    VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to begin the command buffer!\n"
    );
}

std::vector<VkImage> Application::getSwapChainImages(){
    auto swapchain_images_ret = _VulkanParameters._SwapChain.get_images();
    if(!swapchain_images_ret){
        cr::ErrorHandler::handle(
            __FILE__, __LINE__,
            cr::ErrorCode::VULKAN_ERROR,
            "Failed to fetch the swapchain images!\n"
        );
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
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to submit the queue!\n"
    );
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
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to present the queue!\n"
    );
}

}