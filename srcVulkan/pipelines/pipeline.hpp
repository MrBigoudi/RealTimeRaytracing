#pragma once

#include <slang-com-ptr.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <memory>

namespace vkr {

class Pipeline;
using PipelinePtr = std::shared_ptr<Pipeline>;

struct SlangParameters;
struct VulkanAppParameters;
struct DescriptorAllocator;
struct AllocatedImage;

class Pipeline {
    protected:
        VkPipeline _Pipeline{};
        VkPipelineLayout _PipelineLayout{};
        std::vector<VkDescriptorSet> _Descriptors{};
        std::vector<VkDescriptorSetLayout> _DescriptorLayouts{};

    public:
        static const std::string COMPILED_SHADER_DIRECTORY;

    public:
        static bool loadShaderModule(Slang::ComPtr<slang::IBlob> spirvCode, VkDevice device, VkShaderModule* outShaderModule);

        virtual void init(const SlangParameters& slangParamaters, const VulkanAppParameters& vulkanParameters) = 0;
        virtual void destroy(const VulkanAppParameters& vulkanParameters) = 0;

        virtual void initDescriptors(const VulkanAppParameters& vulkanParameters, const AllocatedImage& image, DescriptorAllocator& descriptorAllocator) = 0;
        virtual void destroyDescriptors(const VulkanAppParameters& vulkanParameters, DescriptorAllocator& descriptorAllocator) = 0;

        virtual void run(const VulkanAppParameters& vulkanParameters, const VkCommandBuffer& cmd) = 0;

        VkPipeline getPipeline() const;
        VkPipelineLayout getPipelineLayout() const;
        std::vector<VkDescriptorSet> getDescriptors() const;
        std::vector<VkDescriptorSetLayout> getDescriptorLayouts() const;
        

};

}