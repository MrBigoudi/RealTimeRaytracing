#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.hpp"

namespace vkr{

Application::Application(){
    init();
}

Application::~Application(){
    cleanup();
}

void Application::init(){
    initGLFW();
    initWindow();
    initVulkanParameters();
}

void Application::cleanup(){
    destroyVulkanParameters();
    destroyWindow();
    destroyGLFW();
}

void Application::drawBackground(VkCommandBuffer cmd){
	VkClearColorValue clearValue = _Parameters._BackgroundColor;
	VkImageSubresourceRange clearRange = imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
	//clear image
	vkCmdClearColorImage(cmd, _DrawImage._Image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    // bind the gradient drawing compute pipeline
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipeline);

	// bind the descriptor set containing the draw image for the compute pipeline
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipelineLayout, 0, 1, &_DrawImageDescriptors, 0, nullptr);

	// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
	vkCmdDispatch(cmd, std::ceil(_VulkanParameters._DrawExtent.width / 16.0), std::ceil(_VulkanParameters._DrawExtent.height / 16.0), 1);
}

void Application::draw(){
    // wait until the gpu has finished rendering the last frame. 
    waitFences();
    resetFences();
    //request image from the swapchain
	uint32_t swapchainImageIndex = acquireNextImage();
	
    VkCommandBuffer cmd = getCurrentFrame()._MainCommandBuffer;
	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	resetCommandBuffer(cmd);
	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	//start the command buffer recording
    _VulkanParameters._DrawExtent.width = _DrawImage._ImageExtent.width;
    _VulkanParameters._DrawExtent.height = _DrawImage._ImageExtent.height;
	beginCommandBuffer(cmd, cmdBeginInfo);

    // transition our main draw image into general layout so we can write into it
	// we will overwrite it all so we dont care about what was the older layout
    std::vector<VkImage> swapchainImages = getSwapChainImages();
	transitionImage(cmd, _DrawImage._Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	drawBackground(cmd);

	//transition the draw image and the swapchain image into their correct transfer layouts
	transitionImage(cmd, _DrawImage._Image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	transitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// execute a copy from the draw image into the swapchain
	copyImageToImage(cmd, _DrawImage._Image, swapchainImages[swapchainImageIndex], _VulkanParameters._DrawExtent, _VulkanParameters._SwapChain.extent);

	// set swapchain image layout to Present so we can show it on the screen
	transitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
    endCommandBuffer(cmd);

    //prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished
	VkCommandBufferSubmitInfo cmdinfo = commandBufferSubmitInfo(cmd);	
	VkSemaphoreSubmitInfo waitInfo = semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,getCurrentFrame()._SwapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._RenderSemaphore);	
	VkSubmitInfo2 submit = submitInfo(&cmdinfo,&signalInfo,&waitInfo);	

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	queueSubmit2(&submit);

    //prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = getPresentInfo(&swapchainImageIndex);
    queuePresent(&presentInfo);

	//increase the number of frames drawn
	_FrameNumber++;
}

void Application::processInput(){
    if(glfwGetKey(_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(_Window, GLFW_TRUE);
    }
}

void Application::run(){
    while(!glfwWindowShouldClose(_Window)){
        processInput();
        draw();
        glfwPollEvents();
    }
}

}