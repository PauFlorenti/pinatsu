#pragma once

#include "defines.h"
#include "../vulkanTypes.h"

void
vulkanDeferredShaderCreate(
    const VulkanDevice& device,
    u32 width,
    u32 height,
    VulkanDeferredShader* outShader);

void
vulkanDeferredShaderDestroy(
    const VulkanDevice& device,
    VulkanDeferredShader& shader);