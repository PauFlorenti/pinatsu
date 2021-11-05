#include "vulkanBackend.h"
#include "vulkanTypes.h"
#include "vulkanDevice.h"
#include "vulkanSwapchain.h"
#include "vulkanRenderpass.h"
#include "vulkanFramebuffer.h"

#include "core\application.h"

#include "platform\platform.h"
#include <vector>
#include <string>
#include <fstream>

static VulkanState state;

VkResult vulkanCreateDebugMessenger(VulkanState* pState);

bool vulkanShaderObjectCreate(VulkanState* pState);

bool vulkanCreateFence(
    VulkanState* pState, 
    VulkanFence* fence, 
    bool signaled);
bool vulkanWaitFence(
    VulkanState* pState,
    VulkanFence* fence);
bool vulkanResetFence(
    VulkanState* pState,
    VulkanFence* fence);

static VkBool32 VKAPI_PTR debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        PERROR(pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        PWARN(pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        PINFO(pCallbackData->pMessage);
        break;
    }
    PWARN("Validation layer: %s", pCallbackData->pMessage);
    return VK_FALSE;
}

bool vulkanBackendInit(const char* appName)
{

    state.clientWidth = Application::getInstance()->m_width;
    state.clientHeight = Application::getInstance()->m_height;

    VkApplicationInfo appInfo = {};
    appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = appName;
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "Pinatsu Engine";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.flags            = 0;

    std::vector<const char*> requiredLayers;

    // Get Instance Layers.
#ifdef DEBUG

    requiredLayers.push_back("VK_LAYER_KHRONOS_validation");

    PDEBUG("Required layers:\n");
    for(const auto reqLay : requiredLayers)
        PDEBUG(reqLay);

    u32 instanceLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
    std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data()));

    for(const char* required : requiredLayers)
    {
        PDEBUG("Checking if required %s layer is available.", required);
        bool found = false;
        for(const auto available : availableInstanceLayers)
        {
            if(strcmp(required, available.layerName) == 0){
                found = true;
                PDEBUG("Found.");
                break;
            }
        }
        if(!found){
            PFATAL("Required %s layer not found!", required);
            return false;
        }
    }
#endif
    createInfo.enabledLayerCount    = requiredLayers.size();
    createInfo.ppEnabledLayerNames  = requiredLayers.data();

    // Get extensions
    std::vector<const char*> requiredExtensions;
    platformSpecificExtensions(requiredExtensions);
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef DEBUG
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    PDEBUG("Required extensions:\n");
    for(const auto reqExt : requiredExtensions)
        PDEBUG("\t%s", reqExt);
#endif

    // Check if available Instance Extensions matches required extensions
    u32 instanceExtensionsCount = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr));
    std::vector<VkExtensionProperties> instanceExtensionsPropertiesVector(instanceExtensionsCount);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, instanceExtensionsPropertiesVector.data()));

    for(const char* const required : requiredExtensions)
    {
        PINFO("Checking if %s is available.", required);
        bool found = false;
        for(const VkExtensionProperties available : instanceExtensionsPropertiesVector)
        {
            if(strcmp(required, available.extensionName) == 0){
                found = true;
                PINFO("Found.");
                break;
            }
        }
        if(!found){
            PFATAL("Required %s extension not found. Shutting down!", required);
            return false;
        }
    }

    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &state.instance));

#ifdef DEBUG
    VK_CHECK(vulkanCreateDebugMessenger(&state));
#endif

    if(!platformSurfaceCreation(&state)) {
        return false;
    }

    if(!createLogicalDevice(&state)){
        return false;
    }

    // Create swapchain
    if(!vulkanSwapchainCreate(&state)){
        return false;
    }

    // Render pass
    if(!vulkanRenderPassCreate(&state)){
        return false;
    }

    // Framebuffers
    for(u32 i = 0; i < state.swapchain.imageViews.size(); ++i){

        // TODO Make modular
        std::vector<VkImageView> attachments = {state.swapchain.imageViews.at(i)};

        vulkanFramebufferCreate(
            &state,
            &state.renderpass,
            state.clientWidth, state.clientHeight,
            static_cast<u32>(attachments.size()),
            attachments,
            &state.swapchain.framebuffers.at(i)
        );
    }


    // Command buffers
    state.commandBuffers.resize(state.swapchain.imageCount);
    for(CommandBuffer& cmd : state.commandBuffers)
    {
        VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.commandBufferCount    = 1;
        allocInfo.commandPool           = state.device.commandPool;
        allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        vkAllocateCommandBuffers(state.device.handle, &allocInfo, &cmd.handle);
    }

    // Sync objects
    state.imageAvailableSemaphores.resize(state.swapchain.maxImageInFlight);
    state.renderFinishedSemaphores.resize(state.swapchain.maxImageInFlight);
    state.frameInFlightFences.resize(state.swapchain.maxImageInFlight);

    for(u32 i = 0; i < state.swapchain.maxImageInFlight; ++i)
    {
        VkSemaphoreCreateInfo info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        if(vkCreateSemaphore(state.device.handle, &info, nullptr, &state.imageAvailableSemaphores.at(i)) != VK_SUCCESS){
            PERROR("Semaphore creation failed.");
            return false;
        }
        if(vkCreateSemaphore(state.device.handle, &info, nullptr, &state.renderFinishedSemaphores.at(i)) != VK_SUCCESS)
        {
            PERROR("Semaphore creation failed.");
            return false;
        }

        if(vulkanCreateFence(&state, &state.frameInFlightFences.at(i), true)){
            PERROR("Fence creation failed");
            return false;
        }
    }

    // Shaders Modules
    if(!vulkanShaderObjectCreate(&state)){
        return false;
    }

    return true;
}

void vulkanBackendOnResize()
{
    vulkanSwapchainRecreate(&state);
}

void vulkanBackendShutdown()
{
    vkDestroyInstance(state.instance, nullptr);
}

bool vulkanBeginFrame()
{

    vkQueueWaitIdle(state.device.graphicsQueue);

    // Wait for the previous frame to finish.

    vkWaitForFences(state.device.handle, 1, &state.frameInFlightFences[state.currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(state.device.handle, 1, &state.frameInFlightFences[state.currentFrame]);

    // Acquire next image index.
    vkAcquireNextImageKHR(
        state.device.handle, 
        state.swapchain.handle, 
        UINT64_MAX, 
        state.imageAvailableSemaphores.at(state.currentFrame),
        0, 
        &state.imageIndex);

    VkCommandBufferBeginInfo cmdBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    VK_CHECK(vkBeginCommandBuffer(state.commandBuffers.at(state.imageIndex).handle, &cmdBeginInfo));

    VkClearValue clearColor = {{1.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderPassBeginInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    info.renderPass         = state.renderpass.handle;
    info.framebuffer        = state.swapchain.framebuffers.at(state.imageIndex).handle;
    info.renderArea.offset  = {0, 0};
    info.renderArea.extent  = state.swapchain.extent;
    info.clearValueCount    = 1;
    info.pClearValues       = &clearColor;
    
    vkCmdBeginRenderPass(state.commandBuffers.at(state.imageIndex).handle, &info, VK_SUBPASS_CONTENTS_INLINE);

    return true;
}

bool vulkanDraw()
{
    vkCmdBindPipeline(state.commandBuffers[state.imageIndex].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, state.graphicsPipeline.pipeline);
    vkCmdDraw(state.commandBuffers[state.imageIndex].handle, 3, 1, 0, 0);
    return true;
}

void vulkanEndFrame()
{
    vkCmdEndRenderPass(state.commandBuffers[state.imageIndex].handle);
    VK_CHECK(vkEndCommandBuffer(state.commandBuffers[state.imageIndex].handle));

    VkPipelineStageFlags pipelineStage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &state.commandBuffers[state.imageIndex].handle;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &state.imageAvailableSemaphores[state.currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &state.renderFinishedSemaphores[state.currentFrame];
    submitInfo.pWaitDstStageMask    = pipelineStage;

    if(vkQueueSubmit(state.device.graphicsQueue, 1, &submitInfo, state.frameInFlightFences[state.currentFrame]) != VK_SUCCESS){
        PERROR("Queue wasn't submitted.");
    }

    // Present swapchain image
    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.pImageIndices       = &state.imageIndex;
    presentInfo.swapchainCount      = 1;
    presentInfo.pSwapchains         = &state.swapchain.handle;
    presentInfo.waitSemaphoreCount  = 1;
    presentInfo.pWaitSemaphores     = &state.renderFinishedSemaphores[state.currentFrame];
    
    vkQueuePresentKHR(state.device.presentQueue, &presentInfo);
    state.currentFrame = (state.currentFrame + 1) % state.swapchain.maxImageInFlight;
}

VkResult vulkanCreateDebugMessenger(VulkanState* pState)
{
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; Add info if needed.
    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugMessengerInfo.pfnUserCallback = debugCallback;
    debugMessengerInfo.pUserData = nullptr;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        pState->instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr){
        return func(pState->instance, &debugMessengerInfo, nullptr, &pState->debugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

bool readShaderFile(std::string filename, std::vector<char>& buffer)
{
    std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);

    if(!file.is_open()){
        PERROR("File %s could not be opened!", filename);
        return false;
    }

    // Check if file has length and if it is multiple of 4.
    // To be a valid binary file to be passed to the shader it must be multiple of 4.
    size_t length = file.tellg();
    if(length == 0 || length % 4 != 0) {
        PERROR("File does not meet requirements.");
        return false;
    }
    buffer.resize(length);
    file.seekg(0);
    file.read(buffer.data(), length);
    file.close();
    return true;
}

bool vulkanShaderObjectCreate(VulkanState* pState)
{
    // Shader hardcoded at the moment.
    // TODO Create a way to read and load shader dynamically.

    std::vector<char> vertexBuffer;
    if(!readShaderFile("./data/vert.spv", vertexBuffer)){
        return false;
    }

    std::vector<char> fragBuffer;
    if(!readShaderFile("./data/frag.spv", fragBuffer)){
        return false;
    }

    VkShaderModuleCreateInfo vertInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    vertInfo.codeSize   = vertexBuffer.size();
    vertInfo.pCode      = reinterpret_cast<const u32*>(vertexBuffer.data());

    VkShaderModuleCreateInfo fragInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    fragInfo.codeSize = fragBuffer.size();
    fragInfo.pCode = reinterpret_cast<u32*>(fragBuffer.data());
    
    VK_CHECK(vkCreateShaderModule(pState->device.handle, &vertInfo, nullptr, &pState->vertexShaderObject.shaderModule));
    VK_CHECK(vkCreateShaderModule(pState->device.handle, &fragInfo, nullptr, &pState->fragmentShaderObject.shaderModule));

    // Create Graphics pipeline

    // Shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    VkPipelineShaderStageCreateInfo vertexStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertexStageInfo.stage   = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module  = pState->vertexShaderObject.shaderModule;
    vertexStageInfo.pName   = "main";
    shaderStages.push_back(vertexStageInfo);

    VkPipelineShaderStageCreateInfo fragmentStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragmentStageInfo.stage     = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module    = pState->fragmentShaderObject.shaderModule;
    fragmentStageInfo.pName     = "main";
    shaderStages.push_back(fragmentStageInfo);

    // Vertex Info
    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyInfo.topology                  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable    = VK_FALSE;

    VkViewport viewport = {};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = pState->clientWidth;
    viewport.height     = pState->clientHeight;
    viewport.maxDepth   = 1.0f;
    viewport.minDepth   = 0.0f;

    VkRect2D scissors = {};
    scissors.offset = {0, 0};
    scissors.extent = pState->swapchain.extent;

    VkPipelineViewportStateCreateInfo viewportInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportInfo.viewportCount  = 1;
    viewportInfo.pViewports     = &viewport;
    viewportInfo.scissorCount   = 1;
    viewportInfo.pScissors      = &scissors;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizationInfo.depthClampEnable          = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable   = VK_FALSE;
    rasterizationInfo.polygonMode               = VK_POLYGON_MODE_FILL;
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
    depthStencilInfo.depthTestEnable    = VK_FALSE;
    depthStencilInfo.stencilTestEnable  = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendInfo.attachmentCount  = 1;
    colorBlendInfo.pAttachments     = &colorBlendAttachment;
    colorBlendInfo.logicOpEnable    = VK_FALSE;

    //VkPipelineDynamicStateCreateInfo dynamicStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    //dynamicStateInfo.

    VkPipelineLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    VK_CHECK(vkCreatePipelineLayout(pState->device.handle, &layoutInfo, nullptr, &pState->graphicsPipeline.layout));

    VkGraphicsPipelineCreateInfo info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    info.stageCount             = static_cast<u32>(shaderStages.size());
    info.pStages                = shaderStages.data();
    info.pVertexInputState      = &vertexInputStateInfo;
    info.pInputAssemblyState    = &inputAssemblyInfo;
    info.pViewportState         = &viewportInfo;
    info.pRasterizationState    = &rasterizationInfo;
    info.pMultisampleState      = &multisampleInfo;
    info.pDepthStencilState     = &depthStencilInfo;
    info.pColorBlendState       = &colorBlendInfo;
    info.pDynamicState          = nullptr;
    info.pTessellationState     = nullptr;
    info.layout                 = pState->graphicsPipeline.layout;
    info.renderPass             = pState->renderpass.handle;
    info.subpass                = 0;

    VK_CHECK(vkCreateGraphicsPipelines(pState->device.handle, VK_NULL_HANDLE, 1, &info, nullptr, &pState->graphicsPipeline.pipeline));

    return true;
}

bool vulkanCreateFence(
    VulkanState* pState,
    VulkanFence* fence,
    bool signaled)
{
    VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if(signaled){
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        fence->signaled = true;
    }

    VK_CHECK(vkCreateFence(state.device.handle, &info, nullptr, &fence->handle));
    return true;
}
bool vulkanWaitFence(
    VulkanState *pState,
    VulkanFence* fence)
{
    if(fence->signaled)
        return true;
    
    
}

bool vulkanResetFence(VulkanFence* fence);