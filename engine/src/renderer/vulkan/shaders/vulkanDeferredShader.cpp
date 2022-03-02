#include "vulkanDeferredShader.h"

#include "memory/pmemory.h"

#include "../vulkanShaderModule.h"
#include "../vulkanVertexDeclaration.h"
#include "../vulkanPipeline.h"
#include "../vulkanRenderpass.h"
#include "../vulkanUtils.h"
#include "../vulkanFramebuffer.h"
#include "../vulkanBuffer.h"

static void
createGbuffers(
    const VulkanDevice& device,
    u32 width,
    u32 height,
    VulkanDeferredShader* shader)
{

    VkSamplerCreateInfo sampler = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sampler.magFilter = VK_FILTER_NEAREST;
    sampler.minFilter = VK_FILTER_NEAREST;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.mipLodBias = 0.0;
    sampler.maxAnisotropy = 1.0f;
    sampler.minLod = 0.0f;
    sampler.maxLod = 1.0;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

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

    VK_CHECK(vkCreateSampler(device.handle, &sampler, nullptr, &shader->gbuf.positonTxt.sampler))
    VK_CHECK(vkCreateSampler(device.handle, &sampler, nullptr, &shader->gbuf.normalTxt.sampler))
    VK_CHECK(vkCreateSampler(device.handle, &sampler, nullptr, &shader->gbuf.albedoTxt.sampler))
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
    const VulkanSwapchain& swapchain,
    VulkanDeferredShader* shader)
{
    VkAttachmentDescription attachmentDescription;

    attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE; // for later use.
    attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachmentDescription.format         = swapchain.format.format;

    VkAttachmentDescription depthAttachmentDescription{};
    // TODO Take the depth format accordingly, now it is hardcoded.
    depthAttachmentDescription.format           = swapchain.depthFormat;
    depthAttachmentDescription.samples          = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDescription.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDescription.finalLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentDescription.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDescription.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference attachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &attachmentReference;
	subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass       = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass       = 0;
    dependency.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask    = 0;
    dependency.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depthDependency{};
    depthDependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
    depthDependency.dstSubpass      = 0;
    depthDependency.srcStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depthDependency.srcAccessMask   = 0;
    depthDependency.dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depthDependency.dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachmentDesc[2] = {attachmentDescription, depthAttachmentDescription};
    VkSubpassDependency subpassDependencies[2] = {dependency, depthDependency};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext			= nullptr;
	renderPassInfo.attachmentCount	= 2;
	renderPassInfo.pAttachments		= attachmentDesc;
	renderPassInfo.subpassCount		= 1;
	renderPassInfo.pSubpasses		= &subpass;
	renderPassInfo.dependencyCount	= 2;
	renderPassInfo.pDependencies	= subpassDependencies;

    VK_CHECK(vkCreateRenderPass(device.handle, &renderPassInfo, nullptr, &shader->lightRenderpass.handle));
}

void
vulkanDeferredShaderCreate(
    const VulkanDevice& device,
    const VulkanSwapchain& swapchain,
    u32 width,
    u32 height,
    VulkanDeferredShader* outShader)
{
    vulkanCreateSemaphore(device, &outShader->geometrySemaphore);

    VkCommandPoolCreateInfo cmdPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    cmdPoolInfo.queueFamilyIndex = device.graphicsQueueIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(device.handle, &cmdPoolInfo, nullptr, &outShader->geometryCmdPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.commandPool = outShader->geometryCmdPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(device.handle, &cmdAllocInfo, &outShader->geometryCmdBuffer.handle));

    vulkanBufferCreate(
        device, 
        sizeof(ViewProjectionBuffer),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &outShader->globalUbo);

    u32 objectMaterialSize = sizeof(VulkanMaterialShaderUBO) * VULKAN_MAX_MATERIAL_COUNT;
    vulkanBufferCreate(
        device, 
        objectMaterialSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &outShader->objectUbo);

    VkDescriptorPoolSize geometryPoolSize[3];
    geometryPoolSize[0].descriptorCount    = 1;
    geometryPoolSize[0].type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    geometryPoolSize[1].descriptorCount = 1;
    geometryPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    geometryPoolSize[2].descriptorCount = 1;
    geometryPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolCreateInfo geometryPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    geometryPoolInfo.maxSets        = 2;
    geometryPoolInfo.poolSizeCount  = 3;
    geometryPoolInfo.pPoolSizes     = geometryPoolSize;

    VK_CHECK(vkCreateDescriptorPool(device.handle, &geometryPoolInfo, nullptr, &outShader->geometryDescriptorPool));

    VkDescriptorSetLayoutBinding bindings[VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT];

    VkDescriptorType descriptorTypes[VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
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

    VK_CHECK(vkCreateDescriptorSetLayout(device.handle, &objectBindingInfo, nullptr, &outShader->objectGeometryDescriptorSetLayout));

    createGbuffers(device, width, height, outShader);
    createGeometryRenderPass(device, outShader);
    createLightRenderPass(device, swapchain, outShader);

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
        &outShader->geometryFramebuffer);

    for(u32 i = 0; i < swapchain.imageViews.size(); ++i){

        // TODO Make modular
        std::vector<VkImageView> attachments = {swapchain.imageViews.at(i), swapchain.depthImage.view};

        vulkanFramebufferCreate(
            device,
            &outShader->lightRenderpass,
            width, height,
            static_cast<u32>(attachments.size()),
            attachments,
            &outShader->lightFramebuffer[i]
        );
    }

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

    // Global set -- Viewprojection matrix
    VkDescriptorSetLayoutBinding globalGeometryBinding{};
    globalGeometryBinding.binding           = 0;
    globalGeometryBinding.descriptorCount   = 1;
    globalGeometryBinding.descriptorType    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalGeometryBinding.stageFlags        = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo geometryLayoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    geometryLayoutInfo.bindingCount = 1;
    geometryLayoutInfo.pBindings    = &globalGeometryBinding;

    VK_CHECK(vkCreateDescriptorSetLayout(device.handle, &geometryLayoutInfo, nullptr, &outShader->globalGeometryDescriptorSetLayout));

    // Per Object set -- material data
    VkDescriptorSetLayoutBinding objectGeometryBinding{};
    objectGeometryBinding.binding = 0;
    objectGeometryBinding.descriptorCount = 1;
    objectGeometryBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    objectGeometryBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

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
    VkDescriptorSetLayout layouts[2] = {
        outShader->globalGeometryDescriptorSetLayout,
        outShader->objectGeometryDescriptorSetLayout
    };

    vulkanCreateGraphicsPipeline(
        device,
        &outShader->geometryRenderpass,
        vtx->size,
        vtx->layout,
        geometryShaderStages.size(),
        geometryShaderStages.data(),
        2,
        layouts,
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
        1,
        &outShader->lightDescriptorSetLayout,
        1,
        &colorBlendAttachment,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        viewport,
        scissors,
        false,
        true,
        &outShader->lightPipeline
    );

    VkDescriptorSetLayout deferredLayouts[3] = {
        outShader->lightDescriptorSetLayout,
        outShader->lightDescriptorSetLayout,
        outShader->lightDescriptorSetLayout
    };

    VkDescriptorSetAllocateInfo descSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descSetAllocInfo.descriptorPool     = outShader->geometryDescriptorPool;
    descSetAllocInfo.descriptorSetCount = 1;
    descSetAllocInfo.pSetLayouts        = &outShader->globalGeometryDescriptorSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(device.handle, &descSetAllocInfo, &outShader->globalGeometryDescriptorSet));

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocInfo.descriptorPool       = outShader->lightDescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount   = static_cast<u32>(swapchain.images.size());
    descriptorSetAllocInfo.pSetLayouts          = deferredLayouts;
    
    VK_CHECK(vkAllocateDescriptorSets(device.handle, &descriptorSetAllocInfo, outShader->lightDescriptorSet));
}

void
vulkanDeferredShaderDestroy(
    const VulkanDevice& device,
    VulkanDeferredShader& shader)
{

    vulkanBufferDestroy(device, shader.globalUbo);
    vulkanBufferDestroy(device, shader.objectUbo);

    vkFreeMemory(device.handle, shader.gbuf.positonTxt.image.memory, nullptr);
    vkFreeMemory(device.handle, shader.gbuf.normalTxt.image.memory, nullptr);
    vkFreeMemory(device.handle, shader.gbuf.albedoTxt.image.memory, nullptr);
    vkDestroyImageView(device.handle, shader.gbuf.positonTxt.image.view, nullptr);
    vkDestroyImageView(device.handle, shader.gbuf.normalTxt.image.view, nullptr);
    vkDestroyImageView(device.handle, shader.gbuf.albedoTxt.image.view, nullptr);
    vkDestroyImage(device.handle, shader.gbuf.positonTxt.image.handle, nullptr);
    vkDestroyImage(device.handle, shader.gbuf.normalTxt.image.handle, nullptr);
    vkDestroyImage(device.handle, shader.gbuf.albedoTxt.image.handle, nullptr);
    vkDestroySampler(device.handle, shader.gbuf.positonTxt.sampler, nullptr);
    vkDestroySampler(device.handle, shader.gbuf.normalTxt.sampler, nullptr);
    vkDestroySampler(device.handle, shader.gbuf.albedoTxt.sampler, nullptr);

    // Geometry destruction
    vkDestroyShaderModule(device.handle, shader.shaderStages[0].shaderModule, nullptr);
    vkDestroyShaderModule(device.handle, shader.shaderStages[1].shaderModule, nullptr);
    vkDestroyShaderModule(device.handle, shader.shaderStages[2].shaderModule, nullptr);
    vkDestroyShaderModule(device.handle, shader.shaderStages[3].shaderModule, nullptr);

    vkDestroyDescriptorPool(device.handle, shader.lightDescriptorPool, nullptr);
    vkDestroyDescriptorPool(device.handle, shader.geometryDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device.handle, shader.globalGeometryDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.handle, shader.objectGeometryDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.handle, shader.lightDescriptorSetLayout, nullptr);

    vkDestroySemaphore(device.handle, shader.geometrySemaphore, nullptr);

    vulkanDestroyGrapchisPipeline(device, &shader.geometryPipeline);
    vulkanDestroyGrapchisPipeline(device, &shader.lightPipeline);

    vkDestroyRenderPass(device.handle, shader.geometryRenderpass.handle, nullptr);
    vkDestroyRenderPass(device.handle, shader.lightRenderpass.handle, nullptr);

    vkDestroyCommandPool(device.handle, shader.geometryCmdPool, nullptr);
    vkDestroyFramebuffer(device.handle, shader.geometryFramebuffer.handle, nullptr);
    for(u32 i = 0; i < 3; ++i) {
        vkDestroyFramebuffer(device.handle, shader.lightFramebuffer[i].handle, nullptr);
    }
}

void
vulkanDeferredUpdateGlobalData(
    const VulkanDevice& device,
    VulkanDeferredShader& shader)
{
    VkDescriptorBufferInfo info;
    info.buffer = shader.globalUbo.handle;
    info.offset = 0;
    info.range = sizeof(ViewProjectionBuffer);

    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.dstArrayElement = 0;
    write.dstBinding = 0;
    write.dstSet = shader.globalGeometryDescriptorSet;
    write.pBufferInfo = &info;

    vkUpdateDescriptorSets(device.handle, 1, &write, 0, nullptr);
};

void
vulkanDeferredUpdateGbuffers(
    const VulkanDevice& device,
    const u32 imageIndex,
    VulkanDeferredShader& shader)
{
    VkDescriptorImageInfo textDescPosition;
    textDescPosition.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    textDescPosition.imageView  = shader.gbuf.positonTxt.image.view;
    textDescPosition.sampler    = shader.gbuf.positonTxt.sampler;

    VkDescriptorImageInfo textDescNormal;
    textDescNormal.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    textDescNormal.imageView    = shader.gbuf.normalTxt.image.view;
    textDescNormal.sampler      = shader.gbuf.normalTxt.sampler;

    VkDescriptorImageInfo textDescAlbedo;
    textDescAlbedo.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    textDescAlbedo.imageView    = shader.gbuf.albedoTxt.image.view;
    textDescAlbedo.sampler      = shader.gbuf.albedoTxt.sampler;

    VkDescriptorImageInfo descInfos[3] = {textDescPosition, textDescNormal, textDescAlbedo};

    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.descriptorType    = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount   = 3;
    write.dstBinding        = 0;
    write.dstArrayElement   = 0;
    write.pImageInfo        = descInfos;
    write.dstSet            = shader.lightDescriptorSet[imageIndex];
    
    vkUpdateDescriptorSets(device.handle, 1, &write, 0, nullptr);
}

bool 
vulkanDeferredShaderGetMaterial(
    VulkanState* pState,
    VulkanDeferredShader* shader,
    Material* m)
{
    m->rendererId = shader->objectBufferIndex;
    shader->objectBufferIndex++;

    shader->objectGeometryGeneration[m->rendererId] = INVALID_ID;
    shader->objectGeometryIds[m->rendererId] = INVALID_ID;

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool        = shader->geometryDescriptorPool;
    allocInfo.descriptorSetCount    = 1;
    allocInfo.pSetLayouts           = &shader->objectGeometryDescriptorSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(pState->device.handle, &allocInfo, &shader->objectGeometryDescriptorSet[m->rendererId]));
    return true;
}

void
vulkanDeferredShaderSetMaterial(
    VulkanState* pState,
    VulkanDeferredShader* shader,
    Material* m)
{
    u32 index = pState->imageIndex;
    VkCommandBuffer cmd = shader->geometryCmdBuffer.handle;

    VkDescriptorSet descriptorSet = shader->objectGeometryDescriptorSet[m->rendererId];
    VkWriteDescriptorSet writes[1];
    u32 descriptorCount = 0;
    u32 descriptorIndex = 0;

    u32 range = sizeof(VulkanMaterialShaderUBO);
    u32 offset = range * m->rendererId;
    VulkanMaterialShaderUBO ubo{};
    ubo.diffuseColor = m->diffuseColor;
    vulkanBufferLoadData(pState->device, shader->objectUbo, offset, range, 0, &ubo);

    if(shader->objectGeometryGeneration[m->rendererId] == INVALID_ID ||
        shader->objectGeometryGeneration[m->rendererId] != m->generation)
    {
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = shader->objectUbo.handle;
        bufferInfo.offset = offset;
        bufferInfo.range = range;

        VkWriteDescriptorSet objectWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        objectWrite.pBufferInfo     = &bufferInfo;
        objectWrite.descriptorCount = 1;
        objectWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        objectWrite.dstArrayElement = 0;
        objectWrite.dstBinding      = 0;
        objectWrite.dstSet          = descriptorSet;

        writes[0] = objectWrite;
        descriptorCount++;

        shader->objectGeometryGeneration[m->rendererId] = m->generation;
    }
    descriptorIndex++;

    if(descriptorCount > 0)
        vkUpdateDescriptorSets(pState->device.handle, descriptorCount, writes, 0, nullptr);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->geometryPipeline.layout, 1, 1, &descriptorSet, 0, nullptr);
    /*
    // If descriptor has not been updated, generate the writes.
    if(materialInstance->descriptorState[descriptorIndex].generations[index] == INVALID_ID || materialInstance->descriptorState[descriptorIndex].generations[index] != m->generation)
    {
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer   = shader->objectUbo.handle;
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
    const u32 samplerCount = 3;
    for(u32 samplerIdx = 0; samplerIdx < samplerCount; ++samplerIdx)
    {
        TextureUse use = shader->samplerUses[samplerIdx];
        Texture* t = nullptr;
        switch (use)
        {
        case TEXTURE_USE_DIFFUSE:
            t = m->diffuseTexture;
            break;
        case TEXTURE_USE_NORMAL:
            t = m->normalTexture;
            break;
        case TEXTURE_USE_METALLIC_ROUGHNESS:
            t = m->metallicRoughnessTexture;
            break;
        default:
            break;
        }

        VkDescriptorImageInfo imageInfo[VULKAN_FORWARD_MATERIAL_SAMPLER_COUNT];
        u32* descriptorGeneration = &materialInstance->descriptorState[descriptorIndex].generations[index];
        u32* descriptorId = &materialInstance->descriptorState[descriptorIndex].ids[index];
        // If descriptor sampler has not been updated.
        if(t && (*descriptorId != t->id || *descriptorGeneration == INVALID_ID || t->generation != *descriptorGeneration))
        {
            VulkanTexture* vulkanTexture = (VulkanTexture*)t->data;

            imageInfo[samplerIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[samplerIdx].imageView = vulkanTexture->image.view;
            imageInfo[samplerIdx].sampler = vulkanTexture->sampler;

            VkWriteDescriptorSet textWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            textWrite.descriptorCount   = 1;
            textWrite.pImageInfo        = &imageInfo[samplerIdx];
            textWrite.descriptorType    = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textWrite.dstArrayElement   = 0;
            textWrite.dstBinding        = descriptorIndex;
            textWrite.dstSet            = descriptorSet;

            writes[descriptorCount] = textWrite;
            descriptorCount++;

            if(t->generation != INVALID_ID) {
                *descriptorGeneration = t->generation;
                *descriptorId = t->id;
            }
        }
        descriptorIndex++;
    }

    if(descriptorCount > 0)
        vkUpdateDescriptorSets(pState->device.handle, descriptorCount, writes, 0, nullptr);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.layout, 1, 1, &descriptorSet, 0, nullptr);
    */
}