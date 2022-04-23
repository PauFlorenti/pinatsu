#pragma once

#include "vulkanTypes.h"

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
    VulkanImage* outImage);

void vulkanCreateImageView(
    const VulkanDevice& device, 
    VulkanImage* image,
    VkFormat format,
    VkImageAspectFlags aspectFlags);

void vulkanImageTransitionLayout(
    const VulkanDevice& device, 
    VulkanImage* image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkCommandBuffer& cmd);