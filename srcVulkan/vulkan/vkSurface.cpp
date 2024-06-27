#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.hpp"

namespace vkr{

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

}