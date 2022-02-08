#include "vulkanBuffer.h"

#include "vulkanCommandBuffer.h"
#include "vulkanUtils.h"

bool vulkanBufferCreate(
    VulkanState* pState,
    u32 size,
    u32 usageFlags,
    u32 memFlags,
    VulkanBuffer* buffer)
{
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size                   = size;
    info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE; // It is only used in one queue
    info.usage                  = usageFlags;

    VK_CHECK(vkCreateBuffer(pState->device.handle, &info, nullptr, &buffer->handle));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(pState->device.handle, buffer->handle, &requirements);

    i32 index = findMemoryIndex(pState, requirements.memoryTypeBits, memFlags);
    if(index == -1){
        PERROR("Could not find a valid memory index.");
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize    = requirements.size;
    allocInfo.memoryTypeIndex   = index;
    VK_CHECK(vkAllocateMemory(pState->device.handle, &allocInfo, nullptr, &buffer->memory));
    VK_CHECK(vkBindBufferMemory(pState->device.handle, buffer->handle, buffer->memory, 0));

    return true;
}

/**
 * Upload data to the given buffer by mapping it.
 */
void vulkanBufferLoadData(
    VulkanState* pState,
    VulkanBuffer& buffer,
    VkDeviceSize offset,
    u64 size,
    VkMemoryMapFlags flags,
    const void* data)
{
    void* targetData;
    vkMapMemory(pState->device.handle, buffer.memory, offset, size, flags, &targetData);
    std::memcpy(targetData, data, size);
    vkUnmapMemory(pState->device.handle, buffer.memory);
}

/**
 * @brief Receives a buffer to destroy
 * @param VulkanState& pState
 * @param VkBuffer& buffer to destroy
 * @return void
 */
void vulkanBufferDestroy(
    VulkanState* pState,
    VulkanBuffer& buffer)
{
    vkFreeMemory(
        pState->device.handle, 
        buffer.memory, 
        nullptr);

    vkDestroyBuffer(
        pState->device.handle, 
        buffer.handle,
        nullptr);
}

/**
 * @brief Transfer the buffer info from source buffer to destination buffer.
 * @param VkBuffer src The source of the data to be transferred.
 * @param VkBuffer dst The destination of the data.
 * @param VkDeviceSize size of the data to be transferred.
 * @return void
 */
void vulkanTransferBuffer(
    VulkanState* pState,
    VkBuffer& src,
    VkBuffer& dst,
    VkDeviceSize size)
{
    VkCommandBuffer cmd;
    vulkanCommandBufferAllocateAndBeginSingleUse(
        pState, 
        pState->device.transferCmdPool, 
        cmd);

    VkBufferCopy region;
    region.srcOffset    = 0;
    region.dstOffset    = 0;
    region.size         = size;
    vkCmdCopyBuffer(cmd, src, dst, 1, &region);

    vulkanCommandBufferEndSingleUse(
        pState, 
        pState->device.transferCmdPool, 
        pState->device.transferQueue, 
        cmd);
}

/**
 * @brief Upload data to the GPU through a staging buffer.
 */
void vulkanUploadDataToGPU(
    VulkanState* pState,
    VulkanBuffer& buffer, 
    u32 offset, 
    u64 size, 
    const void* data)
{
    VulkanBuffer stagingBuffer;

    vulkanBufferCreate(
        pState,
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer
    );

    vulkanBufferLoadData(pState, stagingBuffer, 0, size, 0, data);
    vulkanTransferBuffer(pState, stagingBuffer.handle, buffer.handle, size);
    vulkanBufferDestroy(pState, stagingBuffer);
}

/**
 * @brief Copy the buffer data to an image.
 */
void vulkanBufferCopyToImage(
    VulkanState* pState,
    VulkanBuffer* buffer,
    VulkanImage* image,
    VkCommandBuffer& cmd)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {image->width, image->height, 1};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.mipLevel = 0;

    vkCmdCopyBufferToImage(
        cmd, 
        buffer->handle, 
        image->handle, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        1, 
        &region);
}