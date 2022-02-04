#include "vulkanForwardShader.h"

#include "../vulkanBuffer.h"
#include "../vulkanPipeline.h"
#include "../vulkanShaderModule.h"
#include "memory/pmemory.h"

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

    std::vector<char> vertexBuffer;
    if(!readShaderFile("./data/vert.spv", vertexBuffer)){
        return false;
    }

    std::vector<char> fragBuffer;
    if(!readShaderFile("./data/frag.spv", fragBuffer)){
        return false;
    }

    vulkanCreateShaderModule(pState, vertexBuffer, &outShader->shaderStages[0].shaderModule);
    vulkanCreateShaderModule(pState, fragBuffer, &outShader->shaderStages[1].shaderModule);

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

    // Objects descriptor pool and layout
    // Also creates the buffer holding all possible object materials.

    u32 objectMaterialSize = sizeof(VulkanMaterialShaderUBO) * VULKAN_MAX_MATERIAL_COUNT;
    vulkanBufferCreate(
        pState, 
        objectMaterialSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &outShader->meshInstanceBuffer);

    VkDescriptorPoolSize objectDescriptorPoolSize[2];
    objectDescriptorPoolSize[0].descriptorCount    = VULKAN_MAX_MATERIAL_COUNT;
    objectDescriptorPoolSize[0].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    objectDescriptorPoolSize[1].descriptorCount    = VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT * VULKAN_MAX_MATERIAL_COUNT;
    objectDescriptorPoolSize[1].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolCreateInfo objectDescriptorPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    objectDescriptorPoolInfo.poolSizeCount  = 2;
    objectDescriptorPoolInfo.pPoolSizes     = objectDescriptorPoolSize;
    objectDescriptorPoolInfo.maxSets        = VULKAN_MAX_MATERIAL_COUNT;
    objectDescriptorPoolInfo.flags          = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(vkCreateDescriptorPool(pState->device.handle, &objectDescriptorPoolInfo, nullptr, &outShader->meshInstanceDescriptorPool));

    VkDescriptorSetLayoutBinding bindings[VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT];

    VkDescriptorType descriptorTypes[VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    };

    memZero(bindings, sizeof(VkDescriptorSetLayoutBinding) * VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT);
    for(u32 i = 0; i < VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT; ++i)
    {
        bindings[i].binding = i;
        bindings[i].descriptorCount = 1;
        bindings[i].descriptorType = descriptorTypes[i];
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo objectBindingInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    objectBindingInfo.bindingCount  = VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT;
    objectBindingInfo.pBindings     = bindings;

    VK_CHECK(vkCreateDescriptorSetLayout(pState->device.handle, &objectBindingInfo, nullptr, &outShader->meshInstanceDescriptorSetLayout));

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
    viewport.x          = 0;
    viewport.y          = pState->clientHeight;
    viewport.width      = pState->clientWidth;
    viewport.height     = -(f32)pState->clientHeight;
    viewport.maxDepth   = 1;
    viewport.minDepth   = 0;

    VkRect2D scissors;
    scissors.extent = {pState->clientWidth, pState->clientHeight};
    scissors.offset = {0, 0};

    const i32 descriptorSetLayoutCount = 2;
    VkDescriptorSetLayout layouts[descriptorSetLayoutCount] = {
        outShader->globalDescriptorSetLayout,
        outShader->meshInstanceDescriptorSetLayout
    };

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
    uvs.binding     = 0;
    uvs.location    = 2;
    uvs.format      = VK_FORMAT_R32G32_SFLOAT;
    uvs.offset      = sizeof(f32) * 7;

    VkVertexInputAttributeDescription normal{};
    normal.binding  = 0;
    normal.location = 3;
    normal.format   = VK_FORMAT_R32G32B32_SFLOAT;
    normal.offset   = sizeof(f32) * 9;

    const u32 attributeSize = 4;
    VkVertexInputAttributeDescription attributeDescription[attributeSize] = {vert, color, uvs, normal};

    vulkanCreateGraphicsPipeline(
        pState,
        &pState->renderpass,
        attributeSize,
        attributeDescription,
        shaderStages.size(),
        shaderStages.data(),
        descriptorSetLayoutCount,
        layouts,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        viewport,
        scissors,
        false,
        true,
        &outShader->pipeline
    );

    VkDescriptorSetLayout globalLayouts[3] = {
        outShader->globalDescriptorSetLayout,
        outShader->globalDescriptorSetLayout,
        outShader->globalDescriptorSetLayout
    };

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocInfo.descriptorPool       = outShader->globalDescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount   = static_cast<u32>(pState->swapchain.images.size());
    descriptorSetAllocInfo.pSetLayouts          = globalLayouts;
    
    VK_CHECK(vkAllocateDescriptorSets(pState->device.handle, &descriptorSetAllocInfo, outShader->globalDescriptorSet));
    return true;
}

void
vulkanDestroyForwardShader(VulkanState* pState)
{
    vulkanBufferDestroy(pState, pState->forwardShader.globalUbo);
    vulkanBufferDestroy(pState, pState->forwardShader.meshInstanceBuffer);

    vkDestroyShaderModule(pState->device.handle, pState->forwardShader.shaderStages[0].shaderModule, nullptr);
    vkDestroyShaderModule(pState->device.handle, pState->forwardShader.shaderStages[1].shaderModule, nullptr);

    // TO free the descriptor sets -> descriptor pool should have been created with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    // Otherwise, descriptor sets are freed when descriptor pool is destroyed.
    //vkFreeDescriptorSets(pState->device.handle, pState->forwardShader.globalDescriptorPool, 3, pState->forwardShader.globalDescriptorSet);
    vkDestroyDescriptorPool(pState->device.handle, pState->forwardShader.globalDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(pState->device.handle, pState->forwardShader.globalDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(pState->device.handle, pState->forwardShader.meshInstanceDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(pState->device.handle, pState->forwardShader.meshInstanceDescriptorSetLayout, nullptr);

    vkDestroyPipeline(pState->device.handle, pState->forwardShader.pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(pState->device.handle, pState->forwardShader.pipeline.layout, nullptr);
}

void 
vulkanForwardShaderUpdateGlobalData(VulkanState* pState)
{
    VkDescriptorBufferInfo globalUboInfo{};
    globalUboInfo.buffer    = pState->forwardShader.globalUbo.handle;
    globalUboInfo.offset    = 0;
    globalUboInfo.range     = sizeof(ViewProjectionBuffer);

    // Write global shader descriptor
    VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.dstBinding        = 0;
    write.dstArrayElement   = 0;
    write.descriptorCount   = 1;
    write.descriptorType    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo       = &globalUboInfo;
    write.dstSet            = pState->forwardShader.globalDescriptorSet[pState->imageIndex];

    vkUpdateDescriptorSets(pState->device.handle, 1, &write, 0, nullptr);

    VkDeviceSize offset = {0};
    vkCmdBindDescriptorSets(
        pState->commandBuffers[pState->imageIndex].handle, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pState->forwardShader.pipeline.layout,
        0,
        1,
        &pState->forwardShader.globalDescriptorSet[pState->imageIndex],
        0, nullptr);
}

bool vulkanForwardShaderGetMaterial(
    VulkanState* pState,
    VulkanForwardShader* shader,
    Material* m)
{
    m->rendererId = shader->meshInstanceBufferIndex;
    shader->meshInstanceBufferIndex++;

    VulkanMaterialInstance* matInstance = &shader->materialInstances[m->rendererId];
    for(u32 i = 0; i < VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT; ++i)
    {
        for(u32 j = 0; j < 3; ++j)
        {
            matInstance->descriptorState[i].generations[j] = INVALID_ID;
            matInstance->descriptorState[i].ids[j] = INVALID_ID;
        }
    }

    VkDescriptorSetLayout descriptorSetLayouts[3] = {
        shader->meshInstanceDescriptorSetLayout,
        shader->meshInstanceDescriptorSetLayout,
        shader->meshInstanceDescriptorSetLayout
    };

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool        = shader->meshInstanceDescriptorPool;
    allocInfo.descriptorSetCount    = 3; // one per frame
    allocInfo.pSetLayouts           = descriptorSetLayouts;
    VK_CHECK(vkAllocateDescriptorSets(pState->device.handle, &allocInfo, matInstance->descriptorSets));
    return true;
}

void
vulkanForwardShaderSetMaterial(
    VulkanState* pState,
    VulkanForwardShader* shader,
    Material* m)
{
    // Helpers for the index and the command buffer to write to.
    u32 index = pState->imageIndex;
    VkCommandBuffer cmd = pState->commandBuffers[index].handle;

    // Material data
    VulkanMaterialInstance* materialInstance = &shader->materialInstances[m->rendererId];
    VkDescriptorSet descriptorSet = materialInstance->descriptorSets[index];

    // Init writes variables.
    // TODO check if needs update
    VkWriteDescriptorSet writes[VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT];
    memZero(writes, sizeof(VkWriteDescriptorSet) * 2);
    u32 descriptorCount = 0;
    u32 descriptorIndex = 0;

    // Descriptor 0 - Material UBO
    u32 range = sizeof(VulkanMaterialShaderUBO);
    u32 offset = sizeof(VulkanMaterialShaderUBO) * m->rendererId;

    // Upload the data to the ubo.
    VulkanMaterialShaderUBO ubo{};
    ubo.diffuseColor = m->diffuseColor;
    vulkanBufferLoadData(pState, shader->meshInstanceBuffer, offset, range, 0, &ubo);

    // If descriptor has not been updated, generate the writes.
    if(materialInstance->descriptorState[descriptorIndex].generations[index] == INVALID_ID || materialInstance->descriptorState[descriptorIndex].generations[index] != m->generation)
    {
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer   = shader->meshInstanceBuffer.handle;
        bufferInfo.offset   = offset;
        bufferInfo.range    = range;

        VkWriteDescriptorSet objectWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        objectWrite.pBufferInfo     = &bufferInfo;
        objectWrite.descriptorCount = 1;
        objectWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        objectWrite.dstArrayElement = 0;
        objectWrite.dstBinding      = 0;
        objectWrite.dstSet          = descriptorSet;

        writes[descriptorCount] = objectWrite;
        descriptorCount++;

        // Update the generation. 
        materialInstance->descriptorState[descriptorIndex].generations[index] = m->generation;
    }
    descriptorIndex++;

    // Descriptor 1 - Material Sampler ... only diffuse at the moment
    VkDescriptorImageInfo imageInfo;
    Texture* t = m->diffuseTexture;
    u32* descriptorGeneration = &materialInstance->descriptorState[descriptorIndex].generations[index];
    u32* descriptorId = &materialInstance->descriptorState[descriptorIndex].ids[index];
    // If descriptor sampler has not been updated.
    if(t && (*descriptorId != t->id || *descriptorGeneration == INVALID_ID || t->generation != *descriptorGeneration))
    {
        VulkanTexture* vulkanTexture = (VulkanTexture*)t->data;

        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vulkanTexture->image.view;
        imageInfo.sampler = vulkanTexture->sampler;

        VkWriteDescriptorSet textWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        textWrite.descriptorCount = 1;
        textWrite.pImageInfo = &imageInfo;
        textWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textWrite.dstArrayElement = 0;
        textWrite.dstBinding = 1;
        textWrite.dstSet = descriptorSet;

        writes[descriptorCount] = textWrite;
        descriptorCount++;

        if(t->generation != INVALID_ID) {
            *descriptorGeneration = t->generation;
            *descriptorId = t->id;
        }
        descriptorIndex++;
    }

    if(descriptorCount > 0)
        vkUpdateDescriptorSets(pState->device.handle, descriptorCount, writes, 0, nullptr);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.layout, 1, 1, &descriptorSet, 0, nullptr);
}