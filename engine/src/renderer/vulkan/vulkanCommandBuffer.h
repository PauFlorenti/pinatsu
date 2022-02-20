#pragma once

#include "defines.h"
#include "vulkanTypes.h"

//void vulkanCommandBufferFree(VulkanState* pState, VkCommandPool pool, )

void vulkanCommandBufferAllocateAndBeginSingleUse(
    const VulkanDevice& device,
    VkCommandPool pool,
    VkCommandBuffer& cmd);

void vulkanCommandBufferEndSingleUse(
    const VulkanDevice& device,
    VkCommandPool pool,
    VkQueue queue,
    VkCommandBuffer& cmd);