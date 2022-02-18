#include "vulkanDeferredShader.h"

#include "../vulkanShaderModule.h"
#include "../vulkanVertexDeclaration.h"
#include "../vulkanPipeline.h"
#include "../vulkanRenderpass.h"
#include "../vulkanUtils.h"
#include "../vulkanFramebuffer.h"

static void
createGbuffers(
    const VulkanDevice& device,
    u32 width,
    u32 height,
    VulkanDeferredShader* shader)
{
    vulkanCreateAttachment(
        device,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        width, height, &shader->gbuf.positonTxt
    );

    vulkanCreateAttachment(
        device,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        width, height, &shader->gbuf.normalTxt
    );

    vulkanCreateAttachment(
        device,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        width, height, &shader->gbuf.albedoTxt
    );
}


static void
createGeometryRenderPass(
    const VulkanDevice& device,
    VulkanDeferredShader* outShader)
{
   VkAttachmentDescription attachmentDesc[3] = {};

   for(u32 i = 0; i < 3; i++)
   {
       attachmentDesc[i].samples        = VK_SAMPLE_COUNT_1_BIT;
       attachmentDesc[i].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
       attachmentDesc[i].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
       attachmentDesc[i].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
       attachmentDesc[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
       attachmentDesc[i].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
       attachmentDesc[i].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   }

    attachmentDesc[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachmentDesc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attachmentDesc[2].format = VK_FORMAT_R8G8B8A8_UNORM;

    std::vector<VkAttachmentReference> colorReferences;
    colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments		= colorReferences.data();
	subpass.colorAttachmentCount	= static_cast<uint32_t>(colorReferences.size());
	subpass.pDepthStencilAttachment = 0;

	VkSubpassDependency dependencies[2];

	dependencies[0].srcSubpass		= VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass		= 0;
	dependencies[0].srcStageMask	= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask	= VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass		= 0;
	dependencies[1].dstSubpass		= VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask	= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask	= VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext			= nullptr;
	renderPassInfo.pAttachments		= attachmentDesc;
	renderPassInfo.attachmentCount	= 3;
	renderPassInfo.subpassCount		= 1;
	renderPassInfo.pSubpasses		= &subpass;
	renderPassInfo.dependencyCount	= 2;
	renderPassInfo.pDependencies	= dependencies;

    VK_CHECK(vkCreateRenderPass(device.handle, &renderPassInfo, nullptr, &outShader->geometryRenderpass.handle));
}

static void
createLightRenderPass(
    const VulkanDevice& device, 
    VulkanDeferredShader* shader)
{
    VkAttachmentDescription attachmentDescription;

    attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE; // for later use.
    attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachmentDescription.format         = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference attachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &attachmentReference;
	subpass.pDepthStencilAttachment = 0;

    VkSubpassDependency dependency{};
    dependency.srcSubpass       = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass       = 0;
    dependency.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask    = 0;
    dependency.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext			= nullptr;
	renderPassInfo.attachmentCount	= 1;
	renderPassInfo.pAttachments		= &attachmentDescription;
	renderPassInfo.subpassCount		= 1;
	renderPassInfo.pSubpasses		= &subpass;
	renderPassInfo.dependencyCount	= 1;
	renderPassInfo.pDependencies	= &dependency;

    VK_CHECK(vkCreateRenderPass(device.handle, &renderPassInfo, nullptr, &shader->lightRenderpass.handle));
}

void
vulkanDeferredShaderCreate(
    const VulkanDevice& device,
    u32 width,
    u32 height,
    VulkanDeferredShader* outShader)
{

    VkCommandPoolCreateInfo cmdPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    cmdPoolInfo.queueFamilyIndex = device.graphicsQueueIndex;
    VK_CHECK(vkCreateCommandPool(device.handle, &cmdPoolInfo, nullptr, &outShader->geometryCmdPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.commandPool = outShader->geometryCmdPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(device.handle, &cmdAllocInfo, &outShader->geometryCmd.handle));

    createGbuffers(device, width, height, outShader);
    // Create render passes
    createGeometryRenderPass(device, outShader);
    createLightRenderPass(device, outShader);

    std::vector<VkImageView> geometryAttachments = {
        outShader->gbuf.positonTxt.image.view, 
        outShader->gbuf.normalTxt.image.view, 
        outShader->gbuf.albedoTxt.image.view};

    vulkanFramebufferCreate(device,
        &outShader->geometryRenderpass,
        width,
        height,
        geometryAttachments.size(),
        geometryAttachments,
        &outShader->lightFramebuffer);

    // Shader modules
    // Compile hardcoded shaders
    system("glslc ./data/geometry.vert -o ./data/geometry.vert.spv");
    system("glslc ./data/geometry.frag -o ./data/geometry.frag.spv");
    system("glslc ./data/deferredLight.vert -o ./data/deferredLight.vert.spv");
    system("glslc ./data/deferredLight.frag -o ./data/deferredLight.frag.spv");

    std::vector<char> geometryVertex, geometryFragment, deferredVertex, deferredFragment;
    if(!readShaderFile("./data/geometry.vert.spv", geometryVertex)){
        PERROR("Could not read shader!");
    }
    if(!readShaderFile("./data/geometry.frag.spv", geometryFragment)){
        PERROR("Could not read shader!");
    }
    if(!readShaderFile("./data/deferredLight.vert.spv", deferredVertex)){
        PERROR("Could not read shader!");
    }
    if(!readShaderFile("./data/deferredLight.frag.spv", deferredFragment)){
        PERROR("Could not read shader!");
    }

    vulkanCreateShaderModule(device, geometryVertex, &outShader->shaderStages[0].shaderModule);
    vulkanCreateShaderModule(device, geometryFragment, &outShader->shaderStages[1].shaderModule);
    vulkanCreateShaderModule(device, deferredVertex, &outShader->shaderStages[2].shaderModule);
    vulkanCreateShaderModule(device, deferredFragment, &outShader->shaderStages[3].shaderModule);

    VkDescriptorPoolSize geometryPoolSize;
    geometryPoolSize.descriptorCount    = 0;
    //geometryPoolSize.type             = 0; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolCreateInfo geometryPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    geometryPoolInfo.maxSets        = 0;
    geometryPoolInfo.poolSizeCount  = 1;
    geometryPoolInfo.pPoolSizes     = &geometryPoolSize;

    //VK_CHECK(vkCreateDescriptorPool(device.handle, &geometryPoolInfo, nullptr, &outShader->geometryDescriptorPool));

    //VkDescriptorSetLayoutBinding globalGeometryBinding{};
    //globalGeometryBinding.binding           = 0;
    //globalGeometryBinding.descriptorCount   = 1;
    //globalGeometryBinding.descriptorType    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //globalGeometryBinding.stageFlags        = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo geometryLayoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    geometryLayoutInfo.bindingCount = 0;
    geometryLayoutInfo.pBindings    = nullptr;//&globalGeometryBinding;
    geometryLayoutInfo.flags        = 0;

    //VK_CHECK(vkCreateDescriptorSetLayout(device.handle, &geometryLayoutInfo, nullptr, &outShader->globalGeometryDescriptorSetLayout));

    std::vector<VkPipelineShaderStageCreateInfo> geometryShaderStages(2);

    VkPipelineShaderStageCreateInfo geometryVertexShaderStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    geometryVertexShaderStage.module = outShader->shaderStages[0].shaderModule;
    geometryVertexShaderStage.pName = "main";
    geometryVertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    geometryShaderStages.at(0) = geometryVertexShaderStage;

    VkPipelineShaderStageCreateInfo geometryFragmentShaderStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    geometryFragmentShaderStage.module = outShader->shaderStages[1].shaderModule;
    geometryFragmentShaderStage.pName = "main";
    geometryFragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    geometryShaderStages.at(1) = geometryFragmentShaderStage;
    
    VkViewport viewport;
    viewport.x          = 0;
    viewport.y          = height;
    viewport.width      = width;
    viewport.height     = -(f32)height;
    viewport.maxDepth   = 1;
    viewport.minDepth   = 0;

    VkRect2D scissors;
    scissors.extent = {width, height};
    scissors.offset = {0, 0};

    VkPipelineColorBlendAttachmentState blendAttachments[3] = {};
    blendAttachments[0].colorWriteMask = 0xf;
    blendAttachments[0].blendEnable = VK_FALSE;
    blendAttachments[1].colorWriteMask = 0xf;
    blendAttachments[1].blendEnable = VK_FALSE;
    blendAttachments[2].colorWriteMask = 0xf;
    blendAttachments[2].blendEnable = VK_FALSE;

    const VertexDeclaration* vtx = getVertexDeclarationByName("PosColorUvN");

    vulkanCreateGraphicsPipeline(
        device,
        &outShader->geometryRenderpass,
        vtx->size,
        vtx->layout,
        geometryShaderStages.size(),
        geometryShaderStages.data(),
        0, //descriptorSetLayoutCount,
        nullptr, //&outShader->globalGeometryDescriptorSetLayout,// layouts,
        3,
        blendAttachments,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        viewport,
        scissors,
        false,
        true,
        &outShader->geometryPipeline
    );

    // Create light - Presenting pipeline
    VkDescriptorPoolSize lightPoolSize;
    lightPoolSize.type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lightPoolSize.descriptorCount    = 3 * 3;

    VkDescriptorPoolCreateInfo lightPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    lightPoolInfo.maxSets        = 3;
    lightPoolInfo.poolSizeCount  = 1;
    lightPoolInfo.pPoolSizes     = &lightPoolSize;

    VK_CHECK(vkCreateDescriptorPool(device.handle, &lightPoolInfo, nullptr, &outShader->lightDescriptorPool));

    VkDescriptorSetLayoutBinding lightBinding{};
    lightBinding.binding           = 0;
    lightBinding.descriptorCount   = 3;
    lightBinding.descriptorType    = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lightBinding.stageFlags        = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo lightLayoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lightLayoutInfo.bindingCount = 1;
    lightLayoutInfo.pBindings    = &lightBinding;
    lightLayoutInfo.flags        = 0;

    VK_CHECK(vkCreateDescriptorSetLayout(device.handle, &lightLayoutInfo, nullptr, &outShader->lightDescriptorSetLayout));

    std::vector<VkPipelineShaderStageCreateInfo> lightShaderStages(2);

    VkPipelineShaderStageCreateInfo lightVertexShaderStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    lightVertexShaderStage.module = outShader->shaderStages[2].shaderModule;
    lightVertexShaderStage.pName = "main";
    lightVertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    lightShaderStages.at(0) = lightVertexShaderStage;

    VkPipelineShaderStageCreateInfo lightFragmentShaderStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    lightFragmentShaderStage.module = outShader->shaderStages[3].shaderModule;
    lightFragmentShaderStage.pName = "main";
    lightFragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightShaderStages.at(1) = lightFragmentShaderStage;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;

    vulkanCreateGraphicsPipeline(
        device,
        &outShader->lightRenderpass,
        vtx->size,
        vtx->layout,
        lightShaderStages.size(),
        lightShaderStages.data(),
        1, //descriptorSetLayoutCount,
        &outShader->lightDescriptorSetLayout,// layouts,
        1,
        &colorBlendAttachment,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        viewport,
        scissors,
        false,
        true,
        &outShader->lightPipeline
    );

/*
    VkDescriptorSetLayout globalLayouts[3] = {
        outShader->lightDescriptorSetLayout,
        outShader->lightDescriptorSetLayout,
        outShader->lightDescriptorSetLayout
    };
*/
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocInfo.descriptorPool       = outShader->lightDescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount   = 1; //static_cast<u32>(pState->swapchain.images.size());
    descriptorSetAllocInfo.pSetLayouts          = &outShader->lightDescriptorSetLayout;
    
    //VkResult res = vkAllocateDescriptorSets(device.handle, &descriptorSetAllocInfo, &outShader->lightDescriptorSet);
    VK_CHECK(vkAllocateDescriptorSets(device.handle, &descriptorSetAllocInfo, &outShader->lightDescriptorSet));
}

void
vulkanDeferredShaderDestroy(
    const VulkanDevice& device,
    VulkanDeferredShader& shader)
{
    vkFreeMemory(device.handle, shader.gbuf.positonTxt.image.memory, nullptr);
    vkFreeMemory(device.handle, shader.gbuf.normalTxt.image.memory, nullptr);
    vkFreeMemory(device.handle, shader.gbuf.albedoTxt.image.memory, nullptr);
    vkDestroyImageView(device.handle, shader.gbuf.positonTxt.image.view, nullptr);
    vkDestroyImageView(device.handle, shader.gbuf.normalTxt.image.view, nullptr);
    vkDestroyImageView(device.handle, shader.gbuf.albedoTxt.image.view, nullptr);
    vkDestroyImage(device.handle, shader.gbuf.positonTxt.image.handle, nullptr);
    vkDestroyImage(device.handle, shader.gbuf.normalTxt.image.handle, nullptr);
    vkDestroyImage(device.handle, shader.gbuf.albedoTxt.image.handle, nullptr);

    // Geometry destruction
    vkDestroyShaderModule(device.handle, shader.shaderStages[0].shaderModule, nullptr);
    vkDestroyShaderModule(device.handle, shader.shaderStages[1].shaderModule, nullptr);
    vkDestroyShaderModule(device.handle, shader.shaderStages[2].shaderModule, nullptr);
    vkDestroyShaderModule(device.handle, shader.shaderStages[3].shaderModule, nullptr);

    vkDestroyDescriptorPool(device.handle, shader.lightDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device.handle, shader.lightDescriptorSetLayout, nullptr);
    //vkDestroyDescriptorSetLayout(device.handle, shader.globalGeometryDescriptorSetLayout, nullptr);

    vulkanDestroyGrapchisPipeline(device, &shader.geometryPipeline);
    vulkanDestroyGrapchisPipeline(device, &shader.lightPipeline);

    vkDestroyRenderPass(device.handle, shader.geometryRenderpass.handle, nullptr);
    vkDestroyRenderPass(device.handle, shader.lightRenderpass.handle, nullptr);

    vkDestroyCommandPool(device.handle, shader.geometryCmdPool, nullptr);
    vkDestroyFramebuffer(device.handle, shader.lightFramebuffer.handle, nullptr);
}