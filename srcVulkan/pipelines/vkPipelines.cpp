#include "application.hpp"

namespace vkr {

void Application::initPipelines(){
    initGradientPipelines();
}

void Application::destroyPipelines(){
    destroyGradientPipelines();
}

bool Application::loadShaderModule(Slang::ComPtr<slang::IBlob> spirvCode, VkDevice device, VkShaderModule* outShaderModule){

    // create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    // codeSize has to be in bytes, so multply the ints in the buffer by size of
    // int to know the real size of the buffer
    createInfo.codeSize = spirvCode->getBufferSize();
    createInfo.pCode = static_cast<const uint32_t*>(spirvCode->getBufferPointer());

    // check that the creation goes well.
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }
    *outShaderModule = shaderModule;
    return true;
}

}