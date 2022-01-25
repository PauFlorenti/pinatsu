#pragma once

#include "defines.h"
#include "vulkanTypes.h"

bool vulkanBufferCreate(
    VulkanState* pState,
    u32 size,
    u32 usageFlags,
    u32 memFlagss,
    VulkanBuffer* buffer);

void vulkanBufferDestroy(
    VulkanState* pState,
    VulkanBuffer& buffer);

void vulkanTransferBuffer(
    VulkanState* pState,
    VkBuffer &src,
    VkBuffer &dst,
    VkDeviceSize size);

void vulkanBufferLoadData(
    VulkanState* pState,
    VulkanBuffer& buffer,
    VkDeviceSize offset,
    u64 size,
    VkMemoryMapFlags flags,
    const void* data);

void vulkanUploadDataToGPU(
    VulkanState* pState,
    VulkanBuffer& buffer,
    u32 offset,
    u64 size,
    const void* data);

void vulkanBufferCopyToImage(
    VulkanState* pState,
    VulkanBuffer* buffer,
    VulkanImage* image,
    VkCommandBuffer& cmd);