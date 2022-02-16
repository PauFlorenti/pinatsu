#pragma once

#include "defines.h"
#include "../vulkanTypes.h"

struct gbuffers
{
    VkImageView positonTxt;
    VkImageView normalTxt;
    VkImageView albedoTxt;
};

void
vulkanDeferredShaderCreate(
    const VulkanDevice& device,
    u32 width,
    u32 height,
    VulkanDeferredShader* outShader);

void
vulkanDeferredShaderDestroy();