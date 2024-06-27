#include "application.hpp"

namespace vkr{

VkImageCreateInfo Application::imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent){
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
    info.samples = VK_SAMPLE_COUNT_1_BIT;

    //optimal tiling, which means the image is stored on the best gpu format
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

VkImageViewCreateInfo Application::imageviewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags){
    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}

void Application::createImage(VkImageCreateInfo* rimg_info, VmaAllocationCreateInfo* rimg_allocinfo){
	VkResult result = vmaCreateImage(_VulkanParameters._Allocator, rimg_info, rimg_allocinfo, &_DrawImage._Image, &_DrawImage._Allocation, nullptr);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to create an image!\n");
        exit(EXIT_FAILURE);
    }    
}

void Application::createImageView(VkImageViewCreateInfo* rview_info){
	VkResult result = vkCreateImageView(
        _VulkanParameters._Device, 
        rview_info, 
        nullptr, 
        &_DrawImage._ImageView
    );
}

void Application::initImageView(){
    //draw image size will match the window
	VkExtent3D drawImageExtent = {
		_Parameters._WindowWidth,
		_Parameters._WindowHeight,
		1
	};

	//hardcoding the draw format to 32 bit float
	_DrawImage._ImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	_DrawImage._ImageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = imageCreateInfo(_DrawImage._ImageFormat, drawImageUsages, drawImageExtent);

	//for the draw image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	createImage(&rimg_info, &rimg_allocinfo);

	//build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo rview_info = imageviewCreateInfo(_DrawImage._ImageFormat, _DrawImage._Image, VK_IMAGE_ASPECT_COLOR_BIT);
    createImageView(&rview_info);
}

void Application::destroyImageView(){
    vkDestroyImageView(_VulkanParameters._Device, _DrawImage._ImageView, nullptr);
    vmaDestroyImage(_VulkanParameters._Allocator, _DrawImage._Image, _DrawImage._Allocation);
}

}