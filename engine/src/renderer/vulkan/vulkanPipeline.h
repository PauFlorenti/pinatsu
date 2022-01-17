#pragma once

#include "defines.h"
#include "vulkanTypes.h"

/**
 * * Vulkan Graphics pipeline functions
 */

void vulkanCreateGraphicsPipeline(
    VulkanState* pState,
    VulkanRenderpass* renderpass,
    u32 attributeCount,
    VkVertexInputAttributeDescription* attributeDescription,
    u32 stageCount,
    VkPipelineShaderStageCreateInfo* stages,
    u32 descriptorCount,
    VkDescriptorSetLayout* descriptorSetLayouts,
    u32 stride,
    VkViewport viewport,
    VkRect2D scissors,
    bool wireframe,
    bool depthTest,
    VulkanPipeline* outPipeline
);

void vulkanDestroyGrapchisPipeline(
    VulkanState* pState,
    VulkanPipeline* pipeline
);