#include "vulkanDeferredShader.h"

#include "../vulkanShaderModule.h"
#include "../vulkanVertexDeclaration.h"
#include "../vulkanPipeline.h"

void
vulkanDeferredShaderCreate(
    const VulkanDevice& device,
    u32 width,
    u32 height,
    VulkanDeferredShader* outShader)
{
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

    VkPipelineShaderStageCreateInfo geometryVertexShaderStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    geometryVertexShaderStage.module = outShader->shaderStages[0].shaderModule;
    geometryVertexShaderStage.pName = "main";
    geometryVertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineShaderStageCreateInfo geometryFragmentShaderStage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    geometryFragmentShaderStage.module = outShader->shaderStages[1].shaderModule;
    geometryFragmentShaderStage.pName = "main";
    geometryFragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    
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

    const i32 descriptorSetLayoutCount = 2;
    VkDescriptorSetLayout layouts[descriptorSetLayoutCount] = {
        outShader->objectGeometryDescriptorSetLayout,
        outShader->meshInstanceDescriptorSetLayout
    };

    const VertexDeclaration* vtx = getVertexDeclarationByName("PosColorUvN");

    vulkanCreateGraphicsPipeline(
        pState,
        &pState->renderpass,
        vtx->size,
        vtx->layout,
        shaderStages.size(),
        shaderStages.data(),
        descriptorSetLayoutCount,
        layouts,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        viewport,
        scissors,
        false,
        true,
        &outShader->geometryPipeline
    );
}