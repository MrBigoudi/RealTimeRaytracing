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
	beginCommandBuffer(cmd, cmdBeginInfo);

    //make the swapchain image into writeable mode before rendering
    std::vector<VkImage> swapchainImages = getSwapChainImages();
	transitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	VkImageSubresourceRange clearRange = imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

	//clear image
	vkCmdClearColorImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &_Parameters._BackgroundColor, 1, &clearRange);

	//make the swapchain image into presentable mode
	transitionImage(cmd, swapchainImages[swapchainImageIndex],VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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