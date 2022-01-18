#pragma once

#include "defines.h"
#include "vulkanTypes.h"

//void vulkanCommandBufferFree(VulkanState* pState, VkCommandPool pool, )

void vulkanCommandBufferAllocateAndBeginSingleUse(
    VulkanState* pState,
    VkCommandPool pool,
    VkCommandBuffer& cmd);

void vulkanCommandBufferEndSingleUse(
    VulkanState* pState,
    VkCommandPool pool,
    VkQueue queue,
    VkCommandBuffer& cmd);