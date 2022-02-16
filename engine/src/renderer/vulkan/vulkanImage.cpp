#include "vulkanImage.h"

#include "vulkanUtils.h"

void vulkanCreateImage(
    const VulkanDevice& device, 
    VkImageType type,
    u32 width,
    u32 height,
    VkFormat imageFormat,
    VkImageTiling tiling,
    VkImageUsageFlags imageUsage,
    VkMemoryPropertyFlags memoryProperties,
    bool createView,
    VkImageAspectFlags viewAspectFlags,
    VulkanImage* outImage)
{

    outImage->width = width;
    outImage->height = height;

    VkImageCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType      = type;
    info.format         = imageFormat;
    info.extent         = {width, height, 1};
    info.mipLevels      = 1;
    info.arrayLayers    = 1;
    info.tiling         = tiling;
    info.usage          = imageUsage;
    info.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;
    info.samples        = VK_SAMPLE_COUNT_1_BIT;
    //info.queueFamilyIndexCount = 1;
    //info.pQueueFamilyIndices = &pState->device.graphicsQueueIndex; ignored as it is not VK_SHARING_MODE_CONCURRENT
    info.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(device.handle, &info, nullptr, &outImage->handle));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(
        device.handle, 
        outImage->handle, 
        &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryIndex(device, memoryRequirements.memoryTypeBits, memoryProperties);

    VK_CHECK(vkAllocateMemory(device.handle, &memoryAllocateInfo, nullptr, &outImage->memory));

    VK_CHECK(vkBindImageMemory(device.handle, outImage->handle, outImage->memory, 0));

    if(createView)
    {
        outImage->view = 0;
        vulkanCreateImageView(
            device, 
            outImage, 
            imageFormat, 
            viewAspectFlags);
    }
}

void vulkanCreateImageView(
    const VulkanDevice& device, 
    VulkanImage* image,
    VkFormat format,
    VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo info {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    info.image                          = image->handle;
    info.viewType                       = VK_IMAGE_VIEW_TYPE_2D;  // TODO make configurable
    info.format                         = format;
    info.subresourceRange.aspectMask    = aspectFlags;

    //TODO make configurable
    info.subresourceRange.levelCount        = 1;
    info.subresourceRange.baseMipLevel      = 0;
    info.subresourceRange.layerCount        = 1;
    info.subresourceRange.baseArrayLayer    = 0;

    VK_CHECK(vkCreateImageView(device.handle, &info, nullptr, &image->view));
}

void vulkanImageTransitionLayout(
    const VulkanDevice& device, 
    VulkanImage* image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkCommandBuffer& cmd)
{
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.image = image->handle;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = device.graphicsQueueIndex;
    barrier.dstQueueFamilyIndex = device.graphicsQueueIndex;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}