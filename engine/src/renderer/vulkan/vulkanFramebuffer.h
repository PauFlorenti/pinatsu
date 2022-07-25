#pragma once

#include "vulkanTypes.h"

bool vulkanRenderTargetCreate(
    const VulkanDevice& device,
    std::vector<VkImageView>& attachments,
    VulkanRenderpass* renderpass,
    u32 width, u32 height,
    VulkanRenderTarget* renderTarget);

void vulkanRenderTargetDestroy(
    const VulkanDevice& device,
    VulkanRenderTarget* target);