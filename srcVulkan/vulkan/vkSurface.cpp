#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.hpp"
#include "errorHandler.hpp"

namespace vkr{

void Application::initSurface(){
    VkResult result = glfwCreateWindowSurface(_VulkanParameters._Instance, _Window, NULL, &_VulkanParameters._Surface);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to create the surface!\n"
    ); 
}

void Application::destroySurface(){
    vkDestroySurfaceKHR(_VulkanParameters._Instance, _VulkanParameters._Surface, nullptr);
}

}