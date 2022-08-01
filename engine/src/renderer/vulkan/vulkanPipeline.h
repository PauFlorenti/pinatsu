#pragma once

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
    std::vector<VkPushConstantRange>& pushConstantRanges,
    u32 blendAttachmentCount,
    VkPipelineColorBlendAttachmentState* blendAttachments,
    u32 stride,
    VkViewport viewport,
    VkRect2D scissors,
    bool wireframe,
    bool depthTest,
    VulkanRenderPipeline* outPipeline
);

/**
 * @brief Destroy the graphics pipeline.
 * @param VulkanPipeline The pipeline to destroy.
 */
void vulkanDestroyGraphicsPipeline(
    const VulkanDevice& device,
    VulkanPipeline* pipeline);

/**
 * @brief Binds the pipeline for use.
 * @param VulkanDevice The vulkan device state to execute orders.
 * @param CommandBuffer Command buffer to assign the bind.
 * @param VkPipelineBindPoint Graphics pipeline bind point.
 * @param VulkanPipeline The pipeline to be bound.
 */
void vulkanBindPipeline(
    const VulkanDevice& device,
    CommandBuffer* cmd,
    VkPipelineBindPoint bindPoint,
    VulkanPipeline* pipeline);