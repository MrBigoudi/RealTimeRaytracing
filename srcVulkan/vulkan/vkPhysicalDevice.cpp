#include "application.hpp"

namespace vkr{

void Application::initPhysicalDevice(){
    vkb::PhysicalDeviceSelector phys_device_selector(_VulkanParameters._Instance); 
    auto physical_device_selector_return = phys_device_selector
        .set_surface(_VulkanParameters._Surface)
        // Add required extensions
        .add_required_extension("VK_KHR_timeline_semaphore")
        // Add wanted extensions
        // .add_desired_extension("VK_KHR_imageless_framebuffer")
        .select();
    if(!physical_device_selector_return) {
        fprintf(
            stderr,
            "Couldn't find a physical device!\n"
        );
        exit(EXIT_FAILURE);
    }
    _VulkanParameters._PhysicalDevice = physical_device_selector_return.value();
}

void Application::destroyPhysicalDevice(){
    // automatically cleaned
    // no need to destroy a physical device
}

}