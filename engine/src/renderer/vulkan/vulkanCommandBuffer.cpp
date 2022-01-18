#include "vulkanCommandBuffer.h"

void 
vulkanCommandBufferAllocateAndBeginSingleUse(
    VulkanState* pState,
    VkCommandPool pool,
    VkCommandBuffer& cmd)
{
    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(pState->device.handle, &allocInfo, &cmd));

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
}

void 
vulkanCommandBufferEndSingleUse(
    VulkanState* pState,
    VkCommandPool pool,
    VkQueue queue,
    VkCommandBuffer& cmd)
{
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    VK_CHECK(vkQueueWaitIdle(queue));

    vkFreeCommandBuffers(pState->device.handle, pool, 1, &cmd);
}