#include "vulkanDeferredShader.h"

#include "../vulkanShaderModule.h"
#include "../vulkanVertexDeclaration.h"
#include "../vulkanPipeline.h"
#include "../vulkanRenderpass.h"
#include "../vulkanUtils.h"

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
        VK_FORMAT_R32G32B32_SFLOAT,
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
    VulkanRenderpass* renderPass,
    VulkanDeferredShader* outShader)
{
   VkAttachmentDescription attachmentDesc[3] = {};

   for(u32 i = 0; i < 3; i++)
   {
       attachmentDesc[i].samples = VK_SAMPLE_COUNT_1_BIT;
       attachmentDesc[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
       attachmentDesc[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
       attachmentDesc[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
       attachmentDesc[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
       attachmentDesc[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
       attachmentDesc[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   }

    attachmentDesc[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachmentDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
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

void
vulkanDeferredShaderCreate(
    const VulkanDevice& device,
    u32 width,
    u32 height,
    VulkanDeferredShader* outShader)
{
    createGbuffers(device, width, height, outShader);
    // Create render pass

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
    geometryPoolSize.descriptorCount    = 1;
    geometryPoolSize.type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolCreateInfo geometryPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    geometryPoolInfo.maxSets        = 2;
    geometryPoolInfo.poolSizeCount  = 1;
    geometryPoolInfo.pPoolSizes     = &geometryPoolSize;

    VK_CHECK(vkCreateDescriptorPool(device.handle, &geometryPoolInfo, nullptr, &outShader->geometryDescriptorPool));

    VkDescriptorSetLayoutBinding globalGeometryBinding{};
    globalGeometryBinding.binding           = 0;
    globalGeometryBinding.descriptorCount   = 1;
    globalGeometryBinding.descriptorType    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalGeometryBinding.stageFlags        = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo geometryLayoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    geometryLayoutInfo.bindingCount = 1;
    geometryLayoutInfo.pBindings    = &globalGeometryBinding;
    geometryLayoutInfo.flags        = 0;

    VK_CHECK(vkCreateDescriptorSetLayout(device.handle, &geometryLayoutInfo, nullptr, &outShader->globalGeometryDescriptorSetLayout));

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

    /*
    const i32 descriptorSetLayoutCount = 2;
    VkDescriptorSetLayout layouts[descriptorSetLayoutCount] = {
        outShader->objectGeometryDescriptorSetLayout,
        outShader->meshInstanceDescriptorSetLayout
    };
    */

    const VertexDeclaration* vtx = getVertexDeclarationByName("PosColorUvN");

    vulkanCreateGraphicsPipeline(
        device,
        &pState->renderpass,
        vtx->size,
        vtx->layout,
        geometryShaderStages.size(),
        geometryShaderStages.data(),
        1, //descriptorSetLayoutCount,
        &outShader->globalGeometryDescriptorSetLayout,// layouts,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        viewport,
        scissors,
        false,
        true,
        &outShader->geometryPipeline
    );
}