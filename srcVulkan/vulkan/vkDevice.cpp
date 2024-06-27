#include "application.hpp"

namespace vkr{

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

}