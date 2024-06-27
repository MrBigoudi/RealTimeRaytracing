#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <span>
#include <string>
#include <memory>

#include "slang.h"
#include "slang-com-ptr.h"

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"

namespace vkr{

struct ApplicationParameters {
    uint32_t _VulkanVersionMajor = 1;
    uint32_t _VulkanVersionMinor = 3;
    uint32_t _VulkanVersionPatch = 0;

    uint32_t _WindowWidth = 1280;
    uint32_t _WindowHeight = 720;
    std::string _WindowTitle = "RayTracing";
    VkClearColorValue _BackgroundColor = {{0.2f, 0.3f, 0.3f, 1.f}};
};

struct VulkanAppParameters {
    vkb::Instance _Instance{};
    VkSurfaceKHR _Surface{};
    vkb::PhysicalDevice _PhysicalDevice{};
    vkb::Device _Device{};
    VkQueue _GraphicsQueue{};
    uint32_t _GraphicsQueueFamilyIndex{};
    vkb::Swapchain _SwapChain{};
    VkExtent2D _DrawExtent{};
    VmaAllocator _Allocator{};
};

struct AllocatedImage {
    VkImage _Image{};
    VkImageView _ImageView{};
    VmaAllocation _Allocation{};
    VkExtent3D _ImageExtent{};
    VkFormat _ImageFormat{};
};

struct FrameData {
    VkCommandPool _CommandPool{};
	VkCommandBuffer _MainCommandBuffer{};
    VkSemaphore _SwapchainSemaphore{};
    VkSemaphore _RenderSemaphore{};
	VkFence _RenderFence{};
};

struct DescriptorLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> _Bindings;
    void addBinding(uint32_t binding, VkDescriptorType type);
    void clear();
    VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator {

    struct PoolSizeRatio{
		VkDescriptorType _Type;
		float _Ratio;
    };

    VkDescriptorPool _Pool;

    void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
    void clearDescriptors(VkDevice device);
    void destroyPool(VkDevice device);

    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

const uint32_t FRAME_OVERLAP = 2;

struct SlangParameters{
    Slang::ComPtr<slang::IGlobalSession> _GlobalSession;
    Slang::ComPtr<slang::ISession> _Session{};
};


class Application;
using ApplicationPtr = std::shared_ptr<Application>;

class Application {
    private:
        ApplicationParameters _Parameters{};
        VulkanAppParameters _VulkanParameters{};
        AllocatedImage _DrawImage{};
        DescriptorAllocator _GlobalDescriptorAllocator{};
        VkDescriptorSet _DrawImageDescriptors{};
        VkDescriptorSetLayout _DrawImageDescriptorLayout{};

        GLFWwindow* _Window = nullptr;
        FrameData _Frames[FRAME_OVERLAP];
        uint32_t _FrameNumber = 0;

        VkPipeline _gradientPipeline{};
        VkPipelineLayout _gradientPipelineLayout{};

        SlangParameters _Slang{};

    public:
        Application();
        ~Application();

    // vulkan init / cleanup
    private:
        void initInstance();
        void destroyInstance();
        void initMemoryAllocator();
        void destroyMemoryAllocator();
        void initSurface();
        void destroySurface();
        void initPhysicalDevice();
        void destroyPhysicalDevice();
        void initDevice();
        void destroyDevice();
        void initGraphicsQueue();
        void destroyGraphicsQueue();
        void initSwapChain(bool recreate = false);
        void destroySwapChain();
        void initImageView();
        void destroyImageView();
        void initCommands();
        void destroyCommands();
        void initSyncStructures();
        void destroySyncStructures();
        void initDescriptors();
        void destroyDescriptors();
        void initSlang();
        void destroySlang();
        void initPipelines();
        void destroyPipelines();

    // vulkan helpers
    private:
        FrameData& getCurrentFrame();
        void waitFences(uint32_t fenceCount = 1, bool shouldWaitAll = true, uint32_t timeoutInNs = 1e9);
        void resetFences(uint32_t fenceCount = 1);
        uint32_t acquireNextImage(uint32_t timeoutInNs = 1e9);
        void resetCommandBuffer(VkCommandBuffer commandBuffer);
        void endCommandBuffer(VkCommandBuffer commandBuffer);
        void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferBeginInfo commandBufferBeginInfo);
        static void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        static VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask);
        VkSubmitInfo2 submitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
	    void queueSubmit2(VkSubmitInfo2* submit);
        VkPresentInfoKHR getPresentInfo(uint32_t* swapchainImageIndex);
        void queuePresent(VkPresentInfoKHR* presentInfo);
        VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
        VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1);
        VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
        VkCommandBufferSubmitInfo commandBufferSubmitInfo(VkCommandBuffer cmd);
        VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);
        VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
        VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
        std::vector<VkImage> getSwapChainImages();
        VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
        VkImageViewCreateInfo imageviewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
        void createImage(VkImageCreateInfo* rimg_info, VmaAllocationCreateInfo* rimg_allocinfo);
        void createImageView(VkImageViewCreateInfo* rview_info);
        static void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
        static bool loadShaderModule(Slang::ComPtr<slang::IBlob> spirvCode, VkDevice device, VkShaderModule* outShaderModule);
        void initBackgroundPipelines();
        void destroyBackgroundPipelines();


    // init stuff
    private:
        void initVulkanParameters();
        void destroyVulkanParameters();
        void initGLFW();
        void initWindow();
        void destroyGLFW();
        void destroyWindow();
    
    // main functions
    private:
        void init();
        void cleanup();
        void drawBackground(VkCommandBuffer cmd);
        void draw();
        void processInput();

    public:
        void run();        

};
}