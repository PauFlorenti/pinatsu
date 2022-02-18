#pragma once

#include "vulkanTypes.h"

bool vulkanFramebufferCreate(
    const VulkanDevice& device,
    VulkanRenderpass* renderpass, 
    u32 width, u32 height, 
    u32 attachmentCount,
    std::vector<VkImageView>& attachments,
    Framebuffer* outFramebuffer);

void vulkanFramebufferDestroy(
    const VulkanDevice& device,
    Framebuffer* framebuffer);