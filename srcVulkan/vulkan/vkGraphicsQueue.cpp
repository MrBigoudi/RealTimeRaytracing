#include "application.hpp"

#include "errorHandler.hpp"

namespace vkr{

void Application::initGraphicsQueue(){
    auto queue_ret = _VulkanParameters._Device.get_queue(vkb::QueueType::graphics);
    if(!queue_ret){
        cr::ErrorHandler::handle(
            __FILE__, __LINE__,
            cr::ErrorCode::VULKAN_ERROR,
            "Failed to get the graphics queue!\n"
        );
    }
    auto queue_index_ret = _VulkanParameters._Device.get_queue_index(vkb::QueueType::graphics);
    if(!queue_index_ret){
        cr::ErrorHandler::handle(
            __FILE__, __LINE__,
            cr::ErrorCode::VULKAN_ERROR,
            "Failed to get the graphics queue family index!\n"
        );
    }
    _VulkanParameters._GraphicsQueue = queue_ret.value();
    _VulkanParameters._GraphicsQueueFamilyIndex = queue_index_ret.value();
}

void Application::destroyGraphicsQueue(){
    // automatically cleaned
}

}