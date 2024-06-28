#pragma once

#include "pipeline.hpp"

namespace vkr{

class GradientPipeline: public Pipeline {

    public:
        void init(const SlangParameters& slangParamaters, const VulkanAppParameters& vulkanParameters) override;
        void destroy(const VulkanAppParameters& vulkanParameters) override;

        void initDescriptors(const VulkanAppParameters& vulkanParameters, const AllocatedImage& image, DescriptorAllocator& descriptorAllocator) override;
        void destroyDescriptors(const VulkanAppParameters& vulkanParameters, DescriptorAllocator& descriptorAllocator) override;

        void run(const VulkanAppParameters& vulkanParameters, const VkCommandBuffer& cmd) override;


};


}