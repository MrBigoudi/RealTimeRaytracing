#include "application.hpp"

#include "errorHandler.hpp"

namespace vkr{

void Application::initDevice(){
    vkb::DeviceBuilder device_builder{_VulkanParameters._PhysicalDevice};
    auto dev_ret = device_builder.build();
    if(!dev_ret){
        cr::ErrorHandler::handle(
            __FILE__, __LINE__,
            cr::ErrorCode::VULKAN_ERROR,
            "Failed to create the device!\n"
        );
    }
    _VulkanParameters._Device = dev_ret.value();
}

void Application::destroyDevice(){
    vkb::destroy_device(_VulkanParameters._Device);
}

}