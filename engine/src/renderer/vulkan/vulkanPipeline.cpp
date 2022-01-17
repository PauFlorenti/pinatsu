#include "vulkanPipeline.h"

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
    VulkanPipeline* outPipeline)
{
    // Vertex Info
    VkVertexInputBindingDescription vertexBinding = {};
    vertexBinding.binding   = 0;
    vertexBinding.stride    = sizeof(VulkanVertex);
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo   = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputStateInfo.vertexAttributeDescriptionCount        = attributeCount;
    vertexInputStateInfo.pVertexAttributeDescriptions           = attributeDescription;
    vertexInputStateInfo.vertexBindingDescriptionCount          = 1;
    vertexInputStateInfo.pVertexBindingDescriptions             = &vertexBinding;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyInfo.topology                  = (VkPrimitiveTopology)stride;
    inputAssemblyInfo.primitiveRestartEnable    = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportInfo.viewportCount  = 1;
    viewportInfo.pViewports     = &viewport;
    viewportInfo.scissorCount   = 1;
    viewportInfo.pScissors      = &scissors;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizationInfo.depthClampEnable          = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable   = VK_FALSE;
    rasterizationInfo.polygonMode               = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode                  = VK_CULL_MODE_FRONT_BIT;
    rasterizationInfo.frontFace                 = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationInfo.depthBiasEnable           = VK_FALSE;
    rasterizationInfo.depthBiasConstantFactor   = 0.0f;
    rasterizationInfo.depthBiasClamp            = 0.0f;
    rasterizationInfo.depthBiasSlopeFactor      = 0.0f;
    rasterizationInfo.lineWidth                 = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleInfo = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleInfo.rasterizationSamples    = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.sampleShadingEnable     = VK_FALSE;
    multisampleInfo.minSampleShading        = 0.0f;
    multisampleInfo.pSampleMask             = nullptr;
    multisampleInfo.alphaToCoverageEnable   = VK_FALSE;
    multisampleInfo.alphaToOneEnable        = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilInfo.depthTestEnable    = depthTest ? VK_TRUE : VK_FALSE;
    depthStencilInfo.stencilTestEnable  = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendInfo.attachmentCount  = 1;
    colorBlendInfo.pAttachments     = &colorBlendAttachment;
    colorBlendInfo.logicOpEnable    = VK_FALSE;

    std::vector<VkDynamicState> dynamicStates           = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo   = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicStateInfo.dynamicStateCount                  = static_cast<u32>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates                     = dynamicStates.data();

    // Push constant to upload model matrix for each geometry.
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size          = sizeof(mat4);
    pushConstantRange.offset        = 0;
    pushConstantRange.stageFlags    = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutInfo.pushConstantRangeCount   = 1;
    layoutInfo.pPushConstantRanges      = &pushConstantRange;
    layoutInfo.setLayoutCount           = descriptorCount;
    layoutInfo.pSetLayouts              = descriptorSetLayouts;

    VK_CHECK(vkCreatePipelineLayout(pState->device.handle, &layoutInfo, nullptr, &outPipeline->layout));

    VkGraphicsPipelineCreateInfo info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    info.stageCount             = stageCount;
    info.pStages                = stages;
    info.pVertexInputState      = &vertexInputStateInfo;
    info.pInputAssemblyState    = &inputAssemblyInfo;
    info.pViewportState         = &viewportInfo;
    info.pRasterizationState    = &rasterizationInfo;
    info.pMultisampleState      = &multisampleInfo;
    info.pDepthStencilState     = &depthStencilInfo;
    info.pColorBlendState       = &colorBlendInfo;
    info.pDynamicState          = &dynamicStateInfo;
    info.pTessellationState     = nullptr;
    info.layout                 = outPipeline->layout;
    info.renderPass             = renderpass->handle;
    info.subpass                = 0;

    VK_CHECK(vkCreateGraphicsPipelines(pState->device.handle, VK_NULL_HANDLE, 1, &info, nullptr, &outPipeline->pipeline));
}

void vulkanDestroyGrapchisPipeline(
    VulkanState* pState,
    VulkanPipeline* pipeline)
{
    if(!pState || !pipeline){
        return;
    }

    vkDestroyPipelineLayout(
        pState->device.handle,
        pipeline->layout,
        nullptr);
    
    vkDestroyPipeline(
        pState->device.handle,
        pipeline->pipeline,
        nullptr);
}