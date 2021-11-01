#pragma once

#include "vulkanTypes.h"

bool vulkanFramebufferCreate(
    VulkanState* pState, 
    VulkanRenderpass* renderpass, 
    u32 width, u32 height, 
    u32 attachmentCount,
    std::vector<VkImageView>& attachments,
    Framebuffer* outFramebuffer);

void vulkanFramebufferDestroy(
    VulkanState* pState,
    Framebuffer* framebuffer);