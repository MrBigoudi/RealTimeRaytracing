#include "pipeline.hpp"

namespace vkr {

const std::string Pipeline::COMPILED_SHADER_DIRECTORY = std::string(PROJECT_SOURCE_DIR) + "/srcCommon/shaders/compiled/";

bool Pipeline::loadShaderModule(Slang::ComPtr<slang::IBlob> spirvCode, VkDevice device, VkShaderModule* outShaderModule){

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

VkPipeline Pipeline::getPipeline() const{
    return _Pipeline;
}

VkPipelineLayout Pipeline::getPipelineLayout() const{
    return _PipelineLayout;
}

std::vector<VkDescriptorSet> Pipeline::getDescriptors() const {
    return _Descriptors;
}

std::vector<VkDescriptorSetLayout> Pipeline::getDescriptorLayouts() const {
    return _DescriptorLayouts;
}


}