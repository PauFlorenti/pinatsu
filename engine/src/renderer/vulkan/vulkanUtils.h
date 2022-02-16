#pragma once
#include "defines.h"
#include "vulkanTypes.h"

i32 
findMemoryIndex(
    const VulkanDevice& device,
    u32 typeFilter, 
    VkMemoryPropertyFlags memFlags);

/**
 * *Synchronization Utility Functions.
 * This includes both Semaphores and Fences.
 */
 bool vulkanCreateFence(
    const VulkanDevice& device,
    VulkanFence* fence,
    bool signaled);

bool vulkanWaitFence(
    const VulkanDevice& device,
    VulkanFence* fence,
    u64 timeout = UINT64_MAX);
    
bool vulkanResetFence(
    const VulkanDevice& device,
    VulkanFence* fence);

void vulkanDestroyFence(
    const VulkanDevice& device,
    VulkanFence& fence);

bool vulkanCreateSemaphore(
    const VulkanDevice& device,
    VkSemaphore* semaphore);

void vulkanDestroySemaphore(
    const VulkanDevice& device,
    VkSemaphore& semaphore);

/**
 * *Create Attachment
 */
void 
vulkanCreateAttachment(
    const VulkanDevice& device,
    VkFormat format,
    VkImageUsageFlagBits usage,
    u32 width,
    u32 height,
    VulkanTexture* texture);