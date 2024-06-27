#include "application.hpp"

namespace vkr{

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

}