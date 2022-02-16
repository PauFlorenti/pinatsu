#pragma once

#include "defines.h"
#include "../vulkanTypes.h"

struct gbuffers
{
    VulkanTexture positonTxt;
    VulkanTexture normalTxt;
    VulkanTexture albedoTxt;
};

struct VulkanDeferredShader
{
    // 2 for geometry pass and 2 for deferred pass - maybe 2 more for postFX in the future.
    VulkanShaderObject shaderStages[4];
    
    // Geometry pass
    VkDescriptorPool geometryDescriptorPool;
    // Per frame
    VkDescriptorSet globalGeometryDescriptorSet;
    VkDescriptorSetLayout globalGeometryDescriptorSetLayout;
    // Per object
    VkDescriptorSet objectGeometryDescriptorSet;
    VkDescriptorSetLayout objectGeometryDescriptorSetLayout;
    
    VulkanBuffer globalUbo;
    VulkanBuffer objectUbo;

    // Deferred pass
    VkDescriptorSet lightDescriptorSet;
    VkDescriptorSetLayout lightDescriptorSetLayout;

    gbuffers gbuf;

    VulkanPipeline geometryPipeline;
    VulkanPipeline lightPipeline;
    VulkanRenderpass geometryRenderpass;
    VulkanRenderpass lightRenderpass;
};


void
vulkanDeferredShaderCreate(
    const VulkanDevice& device,
    u32 width,
    u32 height,
    VulkanDeferredShader* outShader);

void
vulkanDeferredShaderDestroy();