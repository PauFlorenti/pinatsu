#include "vulkanUtils.h"

i32 
findMemoryIndex(
    const VulkanDevice& device,
    u32 typeFilter, 
    VkMemoryPropertyFlags memFlags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device.physicalDevice, &memProperties);
    for(u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if(typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & memFlags) == memFlags){
            return i;
        }
    }
    return -1;
}

/**
 * *Synchronization Utility Functions.
 * This includes both Semaphores and Fences.
 */

bool vulkanCreateFence(
    const VulkanDevice& device,
    VulkanFence* fence,
    bool signaled)
{
    VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if(signaled){
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        fence->signaled = true;
    } else {
        fence->signaled = false;
    }

    if(vkCreateFence(device.handle, &info, nullptr, &fence->handle) != VK_SUCCESS){
        PERROR("Fence creation failed!");
        return false;
    }
    return true;
};

bool vulkanWaitFence(
    const VulkanDevice& device,
    VulkanFence* fence,
    u64 timeout)
{
    if(fence->signaled)
        return true;
    
    if(vkWaitForFences(
        device.handle, 1, 
        &fence->handle, VK_TRUE, timeout) == VK_SUCCESS)
    {
        return true;
    }
    return false;
}

bool vulkanResetFence(
    const VulkanDevice& device,
    VulkanFence* fence)
{
    if(vkResetFences(
        device.handle, 
        1, 
        &fence->handle) == VK_SUCCESS)
    {
        fence->signaled = false;
        return true;   
    }
    return false;
}

void vulkanDestroyFence(
    const VulkanDevice& device,
    VulkanFence& fence)
{
    vkDestroyFence(
        device.handle,
        fence.handle,
        nullptr);
}

bool vulkanCreateSemaphore(
    const VulkanDevice& device,
    VkSemaphore* semaphore)
{
    VkSemaphoreCreateInfo info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if(vkCreateSemaphore(device.handle, &info, nullptr, semaphore) != VK_SUCCESS){
        PERROR("Semaphore creation failed!");
        return false;
    }
    return true;
}

void vulkanDestroySemaphore(
    const VulkanDevice& device,
    VkSemaphore& semaphore)
{
    vkDestroySemaphore(
        device.handle,
        semaphore,
        nullptr);
}