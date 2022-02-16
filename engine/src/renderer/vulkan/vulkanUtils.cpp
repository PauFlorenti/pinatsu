#include "vulkanUtils.h"

#include "vulkanImage.h"

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
    VulkanTexture* texture)
{

    VkImageAspectFlags aspectFlags;
    VkImageLayout imageLayout;

    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
        imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;        
    }
    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkExtent3D extent = {width, height, 1};
    
    vulkanCreateImage(
        device, 
        VK_IMAGE_TYPE_2D, 
        width, 
        height, 
        format, 
        VK_IMAGE_TILING_OPTIMAL, 
        usage | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        0,
        &texture->image);

}