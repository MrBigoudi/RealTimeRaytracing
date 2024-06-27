#include "application.hpp"

namespace vkr{

void Application::initInstance(){
    vkb::InstanceBuilder instance_builder;
    // configure the instance builder
    instance_builder
        .request_validation_layers()
        .use_default_debug_messenger()
        .set_app_name(_Parameters._WindowTitle.c_str())
        .require_api_version(_Parameters._VulkanVersionMajor,_Parameters._VulkanVersionMinor,_Parameters._VulkanVersionPatch);

    // configure extensions
    auto system_info_ret = vkb::SystemInfo::get_system_info();
    if (!system_info_ret) {
        fprintf(stderr, 
            "Failed to get the system info: %s\n", 
            system_info_ret.error().message().c_str()
        );
        exit(EXIT_FAILURE);
    }
    auto system_info = system_info_ret.value();
    if (system_info.is_layer_available("VK_LAYER_LUNARG_api_dump")) {
        instance_builder.enable_layer("VK_LAYER_LUNARG_api_dump");
    }
    if (system_info.validation_layers_available){
        instance_builder.enable_validation_layers();
    }

    // build the instance
    auto instance_builder_return = instance_builder.build();

    // check errors
    if (!instance_builder_return){
        fprintf(
            stderr, 
            "Failed to create Vulkan Instance: %s\n",
            instance_builder_return.error().message().c_str()
        );
    } 
    // get the instance
    _VulkanParameters._Instance = instance_builder_return.value();
}

void Application::destroyInstance(){
    vkb::destroy_instance(_VulkanParameters._Instance);
}


}