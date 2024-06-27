#include "application.hpp"

namespace vkr{

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

}