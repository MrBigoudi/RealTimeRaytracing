#include "application.hpp"

#include "errorHandler.hpp"

namespace vkr {

void DescriptorAllocator::clearDescriptors(VkDevice device){
    VkResult result = vkResetDescriptorPool(device, _Pool, 0);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to reset the pool!\n"
    );
}

void DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type){
    VkDescriptorSetLayoutBinding newbind {};
    newbind.binding = binding;
    newbind.descriptorCount = 1;
    newbind.descriptorType = type;
    _Bindings.push_back(newbind);
}

void DescriptorLayoutBuilder::clear(){
    _Bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
{
    for (VkDescriptorSetLayoutBinding& b : _Bindings) {
        b.stageFlags |= shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.pNext = pNext;

    info.pBindings = _Bindings.data();
    info.bindingCount = static_cast<uint32_t>(_Bindings.size());
    info.flags = flags;

    VkDescriptorSetLayout set;
    VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &set);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to create the descriptor set layout!\n"
    );

    return set;
}

void DescriptorAllocator::initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios){
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio._Type,
            .descriptorCount = uint32_t(ratio._Ratio * maxSets)
        });
    }

	VkDescriptorPoolCreateInfo pool_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	pool_info.flags = 0;
	pool_info.maxSets = maxSets;
	pool_info.poolSizeCount = (uint32_t)poolSizes.size();
	pool_info.pPoolSizes = poolSizes.data();

	VkResult result = vkCreateDescriptorPool(device, &pool_info, nullptr, &_Pool);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to allocate the descriptor pool!\n"
    );
}

void DescriptorAllocator::destroyPool(VkDevice device){
    vkDestroyDescriptorPool(device,_Pool,nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = _Pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds;
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &ds);
    cr::ErrorHandler::vulkanError(
        result == VK_SUCCESS,
        __FILE__, __LINE__,
        "Failed to allocate the descriptor sets!\n"
    );

    return ds;
}

}