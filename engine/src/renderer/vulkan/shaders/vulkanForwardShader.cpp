#include "vulkanForwardShader.h"

#include "../vulkanBuffer.h"
#include "../vulkanPipeline.h"

/**
 * * Vulkan Shader creation functions
 *  - Create shader stages
 *  - Prepare descriptors for feeding the shader.
 *  - Create the graphics pipeline accordingly.
 */
bool vulkanCreateForwardShader(
    VulkanState* pState,
    VulkanForwardShader* outShader)
{
    // Create the buffer holding the data to upload to the GPU
    vulkanBufferCreate(
        pState, 
        sizeof(ViewProjectionBuffer),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &outShader->globalUbo);

    // Create global descriptor pool
    VkDescriptorPoolSize descriptorPoolSize;
    descriptorPoolSize.descriptorCount  = static_cast<u32>(pState->swapchain.images.size());
    descriptorPoolSize.type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolCreateInfo descriptorPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolInfo.poolSizeCount    = 1;
    descriptorPoolInfo.pPoolSizes       = &descriptorPoolSize;
    descriptorPoolInfo.maxSets          = static_cast<u32>(pState->swapchain.images.size());

    VK_CHECK(vkCreateDescriptorPool(pState->device.handle, &descriptorPoolInfo, nullptr, &outShader->globalDescriptorPool));

    VkDescriptorSetLayoutBinding binding{};
    binding.binding         = 0;
    binding.descriptorCount = 1;
    binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.bindingCount   = 1;
    info.pBindings      = &binding;

    VK_CHECK(vkCreateDescriptorSetLayout(pState->device.handle, &info, nullptr, &outShader->globalDescriptorSetLayout));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);

    VkPipelineShaderStageCreateInfo vertexStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertexStageInfo.stage   = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module  = outShader->shaderStages[0].shaderModule;
    vertexStageInfo.pName   = "main";
    shaderStages.at(0) = (vertexStageInfo);

    VkPipelineShaderStageCreateInfo fragmentStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragmentStageInfo.stage     = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module    = outShader->shaderStages[1].shaderModule;
    fragmentStageInfo.pName     = "main";
    shaderStages.at(1) = (fragmentStageInfo);

    VkViewport viewport;
    viewport.x = 0;
    viewport.y = pState->clientHeight;
    viewport.width = pState->clientWidth;
    viewport.height = -(f32)pState->clientHeight;
    viewport.maxDepth = 1;
    viewport.minDepth = 0;

    VkRect2D scissors;
    scissors.extent = {pState->clientWidth, pState->clientHeight};
    scissors.offset = {0, 0};

    // Position
    VkVertexInputAttributeDescription vert{};
    vert.binding    = 0;
    vert.location   = 0;
    vert.format     = VK_FORMAT_R32G32B32_SFLOAT;
    vert.offset     = 0;

    VkVertexInputAttributeDescription color{};
    color.binding   = 0;
    color.location  = 1;
    color.format    = VK_FORMAT_R32G32B32A32_SFLOAT;
    color.offset    = sizeof(f32) * 3;

    VkVertexInputAttributeDescription uvs{};
    uvs.binding = 0;
    uvs.location = 2;
    uvs.format = VK_FORMAT_R32G32_SFLOAT;
    uvs.offset = sizeof(f32) * 7;

    VkVertexInputAttributeDescription attributeDescription[3] = {vert, color, uvs};

    vulkanCreateGraphicsPipeline(
        pState,
        &pState->renderpass,
        3,
        attributeDescription,
        shaderStages.size(),
        shaderStages.data(),
        1,
        &outShader->globalDescriptorSetLayout,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        viewport,
        scissors,
        false,
        true,
        &outShader->pipeline
    );

    VkDescriptorSetLayout layouts[3] = {
        outShader->globalDescriptorSetLayout,
        outShader->globalDescriptorSetLayout,
        outShader->globalDescriptorSetLayout
    };

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocInfo.descriptorPool       = outShader->globalDescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount   = static_cast<u32>(pState->swapchain.images.size());
    descriptorSetAllocInfo.pSetLayouts          = layouts;
    
    VK_CHECK(vkAllocateDescriptorSets(pState->device.handle, &descriptorSetAllocInfo, outShader->globalDescriptorSet));
    return true;
}

void
vulkanDestroyForwardShader(VulkanState* pState)
{
    vulkanBufferDestroy(pState, pState->forwardShader.globalUbo);

    vkDestroyShaderModule(pState->device.handle, pState->forwardShader.shaderStages[0].shaderModule, nullptr);
    vkDestroyShaderModule(pState->device.handle, pState->forwardShader.shaderStages[1].shaderModule, nullptr);

    // TO free the descriptor sets -> descriptor pool should have been created with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    // Otherwise, descriptor sets are freed when descriptor pool is destroyed.
    //vkFreeDescriptorSets(pState->device.handle, pState->forwardShader.globalDescriptorPool, 3, pState->forwardShader.globalDescriptorSet);
    vkDestroyDescriptorPool(pState->device.handle, pState->forwardShader.globalDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(pState->device.handle, pState->forwardShader.globalDescriptorSetLayout, nullptr);

    vkDestroyPipeline(pState->device.handle, pState->forwardShader.pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(pState->device.handle, pState->forwardShader.pipeline.layout, nullptr);
}