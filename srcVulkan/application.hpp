#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <memory>

#include "VkBootstrap.h"

namespace vkr{

struct ApplicationParameters {
    uint32_t _VulkanVersionMajor = 1;
    uint32_t _VulkanVersionMinor = 3;
    uint32_t _VulkanVersionPatch = 0;

    uint32_t _WindowWidth = 1280;
    uint32_t _WindowHeight = 720;
    uint32_t _ViewportWidth = 1280;
    uint32_t _ViewportHeight = 720;
    std::string _WindowTitle = "RayTracing";
    VkClearColorValue _BackgroundColor = {{0.2f, 0.3f, 0.3f, 1.f}};
};

struct VulkanAppParameters {
    vkb::Instance _Instance;
    VkSurfaceKHR _Surface;
    vkb::PhysicalDevice _PhysicalDevice;
    vkb::Device _Device;
    VkQueue _GraphicsQueue;
    uint32_t _GraphicsQueueFamilyIndex;
    vkb::Swapchain _SwapChain;
};

struct FrameData {
    VkCommandPool _CommandPool;
	VkCommandBuffer _MainCommandBuffer;
    VkSemaphore _SwapchainSemaphore;
    VkSemaphore _RenderSemaphore;
	VkFence _RenderFence;
};

const uint32_t FRAME_OVERLAP = 2;


class Application;
using ApplicationPtr = std::shared_ptr<Application>;

class Application {
    private:
        ApplicationParameters _Parameters{};
        VulkanAppParameters _VulkanParameters{};

        GLFWwindow* _Window = nullptr;
        FrameData _Frames[FRAME_OVERLAP];
        uint32_t _FrameNumber = 0;

    public:
        Application();
        ~Application();

    // vulkan init / cleanup
    private:
        void initInstance();
        void destroyInstance();
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
        void initCommands();
        void destroyCommands();
        void initSyncStructures();
        void destroySyncStructures();

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
        void draw();
        void processInput();

    public:
        void run();        

};
}