#pragma once

#include "defines.h"
#include "../vulkanTypes.h"

void
vulkanDeferredShaderCreate(
    const VulkanDevice& device,
    const VulkanSwapchain& swapchain,
    u32 width,
    u32 height,
    VulkanDeferredShader* outShader);

void
vulkanDeferredShaderDestroy(
    const VulkanDevice& device,
    VulkanDeferredShader& shader);

void
vulkanDeferredUpdateGlobalData(
    const VulkanDevice& device,
    VulkanDeferredShader& shader);

void
vulkanDeferredUpdateGbuffers(
    const VulkanDevice& device,
    const u32 imageIndex,
    VulkanDeferredShader& shader);

bool 
vulkanForwardShaderGetMaterial(
    VulkanState* pState,
    VulkanDeferredShader* shader,
    Material* m);

void
vulkanDeferredShaderSetMaterial(
    VulkanState* pState,
    VulkanDeferredShader* shader,
    Material* m);