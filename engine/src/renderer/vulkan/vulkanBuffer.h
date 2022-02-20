#pragma once

#include "defines.h"
#include "vulkanTypes.h"

bool vulkanBufferCreate(
    const VulkanDevice& device,
    u32 size,
    u32 usageFlags,
    u32 memFlagss,
    VulkanBuffer* buffer);

void vulkanBufferDestroy(
    const VulkanDevice& device,
    VulkanBuffer& buffer);

void vulkanTransferBuffer(
    const VulkanDevice& device,
    VkBuffer &src,
    VkBuffer &dst,
    VkDeviceSize size);

void vulkanBufferLoadData(
    const VulkanDevice& device,
    VulkanBuffer& buffer,
    VkDeviceSize offset,
    u64 size,
    VkMemoryMapFlags flags,
    const void* data);

void vulkanUploadDataToGPU(
    const VulkanDevice& device,
    VulkanBuffer& buffer,
    u32 offset,
    u64 size,
    const void* data);

void vulkanBufferCopyToImage(
    const VulkanDevice& device,
    VulkanBuffer* buffer,
    VulkanImage* image,
    VkCommandBuffer& cmd);