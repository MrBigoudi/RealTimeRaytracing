#include "application.hpp"

namespace vkr {

void Application::initVulkanParameters(){
    initInstance();
    initSurface();
    initPhysicalDevice();
    initDevice();
    initGraphicsQueue();
    initMemoryAllocator();
    initSwapChain();
    initImageView();
    initCommands();
    initSyncStructures();
    initSlang();
    initPipelines();
}

void Application::destroyVulkanParameters(){
    destroyPipelines();
    destroySlang();
    destroySyncStructures();
    destroyCommands();
    destroyImageView();
    destroySwapChain();
    destroyMemoryAllocator();
    destroyGraphicsQueue();
    destroyDevice();
    destroyPhysicalDevice();
    destroySurface();
    destroyInstance();
}

}