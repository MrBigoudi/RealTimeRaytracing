#include "application.hpp"

namespace vkr {

void Application::initDescriptors(){
	//create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes ={
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
	};

	_GlobalDescriptorAllocator.initPool(_VulkanParameters._Device, 10, sizes);

	//make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		_DrawImageDescriptorLayout = builder.build(_VulkanParameters._Device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

    //allocate a descriptor set for our draw image
	_DrawImageDescriptors = _GlobalDescriptorAllocator.allocate(_VulkanParameters._Device, _DrawImageDescriptorLayout);	

	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = _DrawImage._ImageView;
	
	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;
	
	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = _DrawImageDescriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(_VulkanParameters._Device, 1, &drawImageWrite, 0, nullptr);
}

void Application::destroyDescriptors(){
    _GlobalDescriptorAllocator.destroyPool(_VulkanParameters._Device);
    vkDestroyDescriptorSetLayout(_VulkanParameters._Device, _DrawImageDescriptorLayout, nullptr);
}

}

namespace vkr {

void DescriptorAllocator::clearDescriptors(VkDevice device){
    VkResult result = vkResetDescriptorPool(device, _Pool, 0);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to reset the pool!\n");
        exit(EXIT_FAILURE);
    }
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
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to create the descriptor set layout!\n");
        exit(EXIT_FAILURE);
    }

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
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to allocate the descriptor pool!\n");
        exit(EXIT_FAILURE);
    }
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
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to allocate the descriptor sets!\n");
        exit(EXIT_FAILURE);
    }

    return ds;
}

}