#include "application.hpp"

namespace vkr {

void Application::initVulkanParameters(){
    fprintf(stderr, "Init instance...");
    initInstance();
    fprintf(stderr, "\tOk\nInit surface...");
    initSurface();
    fprintf(stderr, "\tOk\nInit physical device...");
    initPhysicalDevice();
    fprintf(stderr, "\tOk\nInit device...");
    initDevice();
    fprintf(stderr, "\tOk\nInit graphics queue...");
    initGraphicsQueue();
    fprintf(stderr, "\tOk\nInit memory allocator...");
    initMemoryAllocator();
    fprintf(stderr, "\tOk\nInit swapchain...");
    initSwapChain();
    fprintf(stderr, "\tOk\nInit image view...");
    initImageView();
    fprintf(stderr, "\tOk\nInit commands...");
    initCommands();
    fprintf(stderr, "\tOk\nInit sync structures...");
    initSyncStructures();
    fprintf(stderr, "\tOk\nInit slang...");
    initSlang();
    fprintf(stderr, "\tOk\nInit pipelines...");
    initPipelines();
    fprintf(stderr, "\tOk\n");
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