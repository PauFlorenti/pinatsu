#pragma once

#include "defines.h"
#include "vulkanTypes.h"

/**
 * * Vulkan Graphics pipeline functions
 */

void vulkanCreateGraphicsPipeline(
    const VulkanDevice& device,
    VulkanRenderpass* renderpass,
    u32 attributeCount,
    const VkVertexInputAttributeDescription* attributeDescription,
    u32 stageCount,
    VkPipelineShaderStageCreateInfo* stages,
    u32 descriptorCount,
    VkDescriptorSetLayout* descriptorSetLayouts,
    u32 blendAttachmentCount,
    VkPipelineColorBlendAttachmentState* blendAttachments,
    u32 stride,
    VkViewport viewport,
    VkRect2D scissors,
    bool wireframe,
    bool depthTest,
    VulkanPipeline* outPipeline
);

void vulkanDestroyGrapchisPipeline(
    const VulkanDevice& device,
    VulkanPipeline* pipeline
);