#pragma once
#include "defines.h"
#include "vulkanTypes.h"

i32 
findMemoryIndex(
    VulkanState* pState,
    u32 typeFilter, 
    VkMemoryPropertyFlags memFlags);