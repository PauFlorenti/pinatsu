#pragma once

#include "defines.h"
#include "vulkanTypes.h"

void vulkanCreateImage(
    VulkanState* pState,
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
    VulkanState* pState, 
    VulkanImage* image,
    VkFormat format,
    VkImageAspectFlags aspectFlags);

void vulkanImageTransitionLayout(
    VulkanState* pState,
    VulkanImage* image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkCommandBuffer& cmd);