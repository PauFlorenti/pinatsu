#include "vulkanBackend.h"

#include "vulkanTypes.h"
#include "vulkanDevice.h"
#include "vulkanSwapchain.h"
#include "vulkanFramebuffer.h"
#include "vulkanBuffer.h"
#include "vulkanCommandBuffer.h"
#include "vulkanImage.h"
#include "vulkanUtils.h"
#include "vulkanImgui.h"
#include "vulkanPlatform.h"
#include "vulkanPipeline.h"
#include "vulkanShaderModule.h"
#include "vulkanVertexDeclaration.h"

#include "core/application.h"

#include "systems/components/comp_transform.h"
#include "systems/components/comp_light_point.h"
#include "systems/components/comp_name.h"
#include "systems/components/comp_camera.h"

#include "memory/pmemory.h"

#define internal static

static f32 gameTime = 0;

static VulkanState state;

/**
 * Vulkan Debug Messenger Functions
 * It has de creation and destruction function.
 * A debug callback is also needed to process the messages
 * sent from the validation layers and treat such messages accordingly.
 */
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

void vulkanDestroyDebugMessenger(VulkanState& state)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(state.instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr){
        func(state.instance, state.debugMessenger, nullptr);
    }
}

/** @brief Compiles a shaders and returns its .spv filename.
 * @param string The name of the shader to compile.
 */
static std::string compileShader(const std::string& name);

void createCommandBuffers();

/**
 * Framebuffers functions
 */
void vulkanRegenerateFramebuffers(
    VulkanSwapchain* swapchain, 
    VulkanRenderpass* renderpass);

bool recreateSwapchain();

// TODO review to function per shader pass.
void vulkanForwardUpdateGlobalState(f32 dt)
{
    gameTime += dt;

    CEntity* hcamera = getEntityByName("camera");
    TCompCamera* cCamera = hcamera->get<TCompCamera>();

    state.boundPipeline->uniformData.view        = cCamera->getView();
    state.boundPipeline->uniformData.projection  = cCamera->getProjection();
    state.boundPipeline->uniformData.position    = cCamera->getEye();

    vulkanBufferLoadData(state.device, state.boundPipeline->uniformBuffer, 0, sizeof(ViewProjectionBuffer), 0, &state.boundPipeline->uniformData);

    vulkanBindGlobals();
    vulkanApplyGlobals();
}

void vulkanLoadPipelines()
{
    json j = loadJson("data/pipelines.json");

    // Create all render passes from json.
    for(auto it : j.items())
    {
        const std::string& key = it.key();
        const json& jdef = it.value();
        std::string name = key + ".pipeline";
        vulkanCreateRenderPassFromJson(name, jdef);
    }
}

void vulkanCreateRenderPassFromJson(const std::string& name, const json& j)
{
    VulkanPipeline* pipeline = nullptr;

    auto it = state.pipelines.find(name.c_str());
    if(it == state.pipelines.end())
    {
        pipeline = new VulkanPipeline();
        state.pipelines[name] = pipeline;
    }
    else{
        pipeline = it->second;
    }

    vulkanCreatePipeline(pipeline, name, j);
}

bool vulkanCreateMesh(Mesh* mesh, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices)
{
    if(!vertexCount || !vertices) {
        PERROR("vulkanCreateMesh - No vertices available. Unable to create mesh.");
        return false;
    }

    bool alreadyUploaded = mesh->rendererId != INVALID_ID;

    // This pointer will point to the internal render mesh.
    VulkanMesh* renderMesh = nullptr;

    if(alreadyUploaded)
    {
        // TODO check old data.
        return true;
    } 
    else
    {
        // If mesh has not been previously uploaded, assign a slot to it.
        for(u32 i = 0; i < VULKAN_MAX_MESHES; ++i) {
            if(state.vulkanMeshes[i].id == INVALID_ID){
                mesh->rendererId = i;
                state.vulkanMeshes[i].id = i;
                renderMesh = &state.vulkanMeshes[i];
                break;
            }
        }
    }

    // Check if a slot was available, otherwise return false.
    if(!renderMesh)
    {
        PFATAL("vulkanCreateMesh - failed to assign an index. No slot available.");
        return false;
    }

    renderMesh->vertexCount   = vertexCount;
    renderMesh->vertexOffset  = 0;
    renderMesh->vertexSize    = sizeof(VulkanVertex);
    u64 totalVertexSize = renderMesh->vertexCount * renderMesh->vertexSize;

    u32 flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vulkanBufferCreate(state.device, totalVertexSize, flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &renderMesh->vertexBuffer);
    vulkanUploadDataToGPU(state.device, renderMesh->vertexBuffer, 0, totalVertexSize, vertices);

    if(indexCount > 0 && indices)
    {
        u64 indexSize = indexCount * sizeof(u32);
        u32 indexFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        renderMesh->indexCount = indexCount;
        vulkanBufferCreate(state.device, indexSize, indexFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &renderMesh->indexBuffer);
        vulkanUploadDataToGPU(state.device, renderMesh->indexBuffer, 0, indexSize, indices);
    }

    return true;
}

void vulkanDestroyMesh(const Mesh* mesh)
{
    if(!mesh) {
        return;
    }

    for(u32 i = 0;
        i < VULKAN_MAX_MESHES;
        ++i)
    {
        if(state.vulkanMeshes[i].id == mesh->id)
        {
            vulkanBufferDestroy(state.device, state.vulkanMeshes[i].vertexBuffer);
            if(state.vulkanMeshes[i].indexBuffer.handle)
            {
                vulkanBufferDestroy(state.device, state.vulkanMeshes[i].indexBuffer);
            }
            state.vulkanMeshes[i].id = INVALID_ID;
            break;
        }
    }
}

bool vulkanCreateTexture(void* pixels, Texture* texture)
{
    if(!pixels || !texture) {
        PERROR("vulkanCreateTexture - Unable to create the texture given the inputs.");
        return false;
    }

    texture->data = memAllocate(sizeof(VulkanTexture), MEMORY_TAG_TEXTURE);
    VulkanTexture* data = (VulkanTexture*)texture->data;
    VkDeviceSize textureSize = texture->width * texture->height * texture->channels;

    // ! Assume 8 bit per channel
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    // Staging buffer, load data into it.
    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memPropsFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VulkanBuffer staging;
    vulkanBufferCreate(state.device, textureSize, usageFlags, memPropsFlags, &staging);
    vulkanBufferLoadData(state.device, staging, 0, textureSize, 0, pixels);

    vulkanCreateImage(
        state.device,
        VK_IMAGE_TYPE_2D,
        texture->width,
        texture->height,
        format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &data->image
    );

    VkCommandBuffer temporalCommand;
    vulkanCommandBufferAllocateAndBeginSingleUse(
        state.device, 
        state.device.commandPool, 
        temporalCommand);

    vulkanImageTransitionLayout(
        state.device, 
        &data->image, 
        format, 
        VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        temporalCommand);

    vulkanBufferCopyToImage(
        state.device,
        &staging, 
        &data->image, 
        temporalCommand);
    
    vulkanImageTransitionLayout(
        state.device, 
        &data->image, 
        format, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
        temporalCommand);

    vulkanCommandBufferEndSingleUse(
        state.device, 
        state.device.commandPool, 
        state.device.graphicsQueue, 
        temporalCommand);

    vulkanBufferDestroy(state.device, staging);

    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VK_CHECK(vkCreateSampler(state.device.handle, &samplerInfo, nullptr, &data->sampler));
    texture->generation++;
    return true;
}

void vulkanDestroyTexture(Texture* texture)
{
    vkDeviceWaitIdle(state.device.handle);

    VulkanTexture* data = (VulkanTexture*)texture->data;
    if(data)
    {
        vkFreeMemory(state.device.handle, data->image.memory, nullptr);
        vkDestroyImage(state.device.handle, data->image.handle, nullptr);
        vkDestroyImageView(state.device.handle, data->image.view, nullptr);
        vkDestroySampler(state.device.handle, data->sampler, nullptr);
        memZero(&data->image, sizeof(VulkanTexture));
    }
    memZero(texture, sizeof(Texture));
}

bool vulkanCreateMaterial(Material* m)
{
    return true;
    /* 
    if(m)
    {
        switch (m->type)
        {
        case MATERIAL_TYPE_FORWARD:
            if(!vulkanForwardShaderGetMaterial(&state, &state.forwardShader, m))
            {
                PERROR("vulkanCreateMaterial - could not create material '%s'.", m->name);
            }
            if(!vulkanDeferredShaderGetMaterial(&state, &state.deferredShader, m))
            {
                PERROR("vulkanCreateMaterial - could not create deferred material '%s'.", m->name);
            }
            break;
        case MATERIAL_TYPE_UI:
            break;
        default:
            break;
        }
        return true;
    }
    return false; */
}

/**
 * @brief Initialize all vulkan render system.
 * @param const char* application name.
 * @return bool if succeded initialization.
 */
bool vulkanBackendInit(const char* appName, void* winHandle)
{
    applicationGetFramebufferSize(&state.clientWidth, &state.clientHeight);

    state.windowHandle = winHandle;

    VkApplicationInfo appInfo = {};
    appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = appName;
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "Pinatsu Engine";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_API_VERSION_1_3;

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
    platformSpecificVulkanExtensions(requiredExtensions);
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

    if(!platformCreateVulkanSurface(&state)) {
        return false;
    }

    if(!createLogicalDevice(&state)){
        return false;
    }

    // Create swapchain
    if(!vulkanSwapchainCreate(&state)){
        return false;
    }

    vulkanLoadPipelines();

    createCommandBuffers();

    // Sync objects
    state.imageAvailableSemaphores.resize(state.swapchain.maxImageInFlight);
    state.renderFinishedSemaphores.resize(state.swapchain.maxImageInFlight);
    state.frameInFlightFences.resize(state.swapchain.maxImageInFlight);

    for(u32 i = 0; i < state.swapchain.maxImageInFlight; ++i)
    {
        if(!vulkanCreateSemaphore(state.device, &state.imageAvailableSemaphores.at(i))){
            return false;
        }
        if(!vulkanCreateSemaphore(state.device, &state.renderFinishedSemaphores.at(i))){
            return false;
        }
        if(!vulkanCreateFence(state.device, &state.frameInFlightFences.at(i), true)){
            return false;
        }
    }

    // Make sure we set the images in use to NULL
    for(u32 i = 0; i < 3; ++i)
        state.imagesInFlight[i] = VK_NULL_HANDLE;

    state.vulkanMeshes = (VulkanMesh*)memAllocate(sizeof(VulkanMesh) * VULKAN_MAX_MESHES, MEMORY_TAG_RENDERER);
    for(u32 i = 0; i < VULKAN_MAX_MESHES; ++i) {
        state.vulkanMeshes[i].id = INVALID_ID;
    }

    imguiInit(&state, &state.pipelines["debug.pipeline"]->renderpass);

    return true;
}

/**
 * @brief Triggered when resized. Take new size and mark the
 * application to be resized.
 * @param u32 width
 * @param u32 height
 * @return void
 */
void vulkanBackendOnResize(u32 width, u32 height)
{
    state.resized       = true;
    state.clientWidth   = width;
    state.clientHeight  = height;
}

/**
 * @brief Shutdown and destroy all modules and components from
 * the vulkan backend. Usually when shutting down the renderer module.
 * @param void
 * @return void
 */
void vulkanBackendShutdown(void)
{
    vkDeviceWaitIdle(state.device.handle);

    // Destroy all synchronization resources
    for(VkSemaphore& semaphore : state.imageAvailableSemaphores)
    {
        vulkanDestroySemaphore(state.device, semaphore);
    }
    for(VkSemaphore& semaphore : state.renderFinishedSemaphores)
    {
        vulkanDestroySemaphore(state.device, semaphore);
    }
    for(VulkanFence& fence : state.frameInFlightFences)
    {
        vulkanDestroyFence(state.device, fence);
    }

    // Destroy all buffers from loaded meshes
    for(u32 i = 0;
        i < VULKAN_MAX_MESHES;
        ++i)
    {
        if(state.vulkanMeshes[i].id != INVALID_ID)
        {
            vulkanBufferDestroy(state.device, state.vulkanMeshes[i].vertexBuffer);
            if(state.vulkanMeshes[i].indexBuffer.handle){
                vulkanBufferDestroy(state.device, state.vulkanMeshes[i].indexBuffer);
            }
        }
    }

    imguiDestroy();


    PDEBUG("Destroying Vulkan Pipelines ...");
    for(std::map<std::string, VulkanPipeline*>::iterator it = state.pipelines.begin(); it != state.pipelines.end(); ++it)
        vulkanPipelineDestroy(it->second);

    //vkDestroyRenderPass(state.device.handle, state.pipelines["debug.pipeline"]->renderpass.handle, nullptr);

    /*
    for(auto framebuffer : state.swapchain.framebuffers)
    {
        vkDestroyFramebuffer(state.device.handle, framebuffer.handle, nullptr);
    }
    */

    PDEBUG("Destroying Vulkan Swapchain ...");
    vulkanSwapchainDestroy(&state);

    PDEBUG("Destroying Vulkan Logical device ...");
    destroyLogicalDevice(state);
    if(state.surface)
    {
        PDEBUG("Destroying Vulkan Surface ...");
        vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
        state.surface = 0;
    }

    // Debug Messenger
#ifdef DEBUG
    if(state.debugMessenger){
        PDEBUG("Destroying Vulkan Debug Messenger ...");
        vulkanDestroyDebugMessenger(state);
    }
#endif

    PDEBUG("Destroying Vulkan instance ...");
    vkDestroyInstance(state.instance, nullptr);
}

/**
 * @brief Do necessary preparation to begin the frame rendering.
 * If failed, vulkanDraw won't be called and rendering pass won't perform.
 * @param void
 * @return bool if succeded.
 */
bool vulkanBeginFrame(f32 delta)
{
    // Wait for the device to finish recreating the swapchain.
    if(state.recreatingSwapchain) {
        if(VK_SUCCESS != vkDeviceWaitIdle(state.device.handle)){
            PERROR("vkDeviceWaitIdle failed!");
            return false;
        }
        PINFO("Recreating swapchain, booting.");
        return false;
    }

    if(state.resized)
    {
        if(!recreateSwapchain())
        {
            PFATAL("Swapchain could not be recreated.");
            return false;
        }
        state.resized = false;
        PINFO("Resized, booting");
        return false;
    }

    // Wait for the previous frame to finish.
    //vulkanResetFence(state.device, &state.frameInFlightFences[state.currentFrame]);
    vulkanWaitFence(
        state.device, 
        &state.frameInFlightFences[state.currentFrame]);

    // Acquire next image index.
    vkAcquireNextImageKHR(
        state.device.handle, 
        state.swapchain.handle, 
        UINT64_MAX, 
        state.imageAvailableSemaphores.at(state.currentFrame),
        0, 
        &state.imageIndex);


    // Start recording the command buffer
    VkCommandBuffer cmd = state.commandBuffers[state.imageIndex].handle;
    VkCommandBufferBeginInfo cmdBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    VkViewport viewport;
    viewport.x          = 0.0f;
    viewport.y          = state.clientHeight;
    viewport.width      = state.clientWidth;
    viewport.height     = -(f32)state.clientHeight;
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor;
    scissor.extent.width    = state.clientWidth;
    scissor.extent.height   = state.clientHeight;
    scissor.offset.x        = 0.0;
    scissor.offset.y        = 0.0;

    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    return true;
}

bool vulkanBeginRenderPass(const std::string& renderpass)
{
    CommandBuffer cmd = state.commandBuffers[state.imageIndex];

    // TODO This should be thought to be changed.
    VulkanRenderpass* renderPass = &state.pipelines[renderpass]->renderpass; //state.renderpasses[renderpass];

    VkRenderPassBeginInfo renderpassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderpassInfo.renderPass = renderPass->handle;
    renderpassInfo.framebuffer = renderPass->renderTargets[state.imageIndex].handle;
    // TODO This should be given by the renderpas.
    renderpassInfo.renderArea.offset = {0, 0};
    renderpassInfo.renderArea.extent = state.swapchain.extent;

    // TODO Should be driven by the renderpass info
    VkClearValue clearColors[2];
    clearColors[0] = {{0.2f, 0.2f, 0.2f, 1.0f}};
    clearColors[1].depthStencil.depth = 1.0f;
    clearColors[1].depthStencil.stencil = 0;

    renderpassInfo.clearValueCount = 2;
    renderpassInfo.pClearValues = clearColors;

    vkCmdBeginRenderPass(cmd.handle, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
    return true;
}

void vulkanDrawGeometry(const RenderMeshData* data)
{
    // TODO make material specify the type to render
    Material* m = data->material;
    if(!m){
        Material mat;
        mat.diffuseColor = glm::vec4(1);
        m = &mat;
    }

    CommandBuffer cmd = state.commandBuffers[state.imageIndex];
    VulkanMesh* geometry = &state.vulkanMeshes[data->mesh->rendererId];

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd.handle, 0, 1, &geometry->vertexBuffer.handle, &offset);
    //vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vkCmdPushConstants(cmd.handle, state.boundPipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &data->model);
    if(geometry->indexCount > 0)
    {
        vkCmdBindIndexBuffer(cmd.handle, geometry->indexBuffer.handle, offset, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd.handle, geometry->indexCount, 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(cmd.handle, geometry->vertexCount, 1, 0, 0);
    }
}

void vulkanEndRenderPass(const std::string& renderPass)
{
    CommandBuffer cmd = state.commandBuffers[state.imageIndex];
    vkCmdEndRenderPass(cmd.handle);
}

/**
 * @brief Ends the render passa and command buffers for this frame.
 * Submits info to the graphics queue and presents the image.
 * @param void
 * @return void
 */
void vulkanEndFrame(void)
{
    // End command buffer.
    VkCommandBuffer cmd = state.commandBuffers[state.imageIndex].handle;
    vkEndCommandBuffer(cmd);

    // We wait for fences only if the previous frame was in use of the image.
    if(state.imagesInFlight[state.imageIndex] != VK_NULL_HANDLE)
        VK_CHECK(vkWaitForFences(state.device.handle, 1, &state.imagesInFlight[state.imageIndex], VK_TRUE, UINT64_MAX));

    state.imagesInFlight[state.imageIndex] = state.frameInFlightFences[state.imageIndex].handle;
    VK_CHECK(vkResetFences(state.device.handle, 1, &state.frameInFlightFences[state.imageIndex].handle));

    VkPipelineStageFlags pipelineStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &state.commandBuffers[state.imageIndex].handle;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &state.imageAvailableSemaphores[state.currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &state.renderFinishedSemaphores[state.currentFrame];
    submitInfo.pWaitDstStageMask    = &pipelineStage;

    if(vkQueueSubmit(state.device.graphicsQueue, 1, &submitInfo, state.frameInFlightFences[state.currentFrame].handle) != VK_SUCCESS){
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

/**
 * @brief Regenerate the framebuffers given a new swapchain
 * @param VulkanSwapchain swapchain
 * @param VulkanRenderpass renderpass
 * @return void
 */
void vulkanRegenerateFramebuffers(
    VulkanSwapchain* swapchain, 
    VulkanRenderpass* renderpass)
{
    for(u8 i = 0; i < state.swapchain.imageViews.size(); ++i)
    {
        std::vector<VkImageView> attachments = {state.swapchain.imageViews.at(i), state.swapchain.depthImage.view};
        VulkanRenderTarget* renderTarget = new VulkanRenderTarget();
        vulkanRenderTargetCreate(state.device, attachments, renderpass, state.swapchain.extent.width, state.swapchain.extent.height, renderTarget);
        renderpass->renderTargets.emplace_back(*renderTarget);
    }
}

/**
 * @brief Function to create main command buffers for main rendering.
 * @param void
 * @return void
 */
void createCommandBuffers()
{
    state.commandBuffers.resize(state.swapchain.imageCount);
    for(CommandBuffer& cmd : state.commandBuffers)
    {
        VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.commandBufferCount    = 1;
        allocInfo.commandPool           = state.device.commandPool;
        allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        vkAllocateCommandBuffers(state.device.handle, &allocInfo, &cmd.handle);
    }
}

/**
 * @brief Recreate the swapchain for cases like minimization or resizing
 * of the application.
 * @param void
 * @return bool if succeded recreating the swapchain.
 */
bool recreateSwapchain()
{
    if(state.recreatingSwapchain){
        PDEBUG("RecreateSwapchain function called before finishing already ongoing recreation.");
        return false;
    }

    state.recreatingSwapchain = true;

    vkDeviceWaitIdle(state.device.handle);

    // Make sure we set the images in use to NULL
    for(u32 i = 0; i < 3; ++i)
        state.imagesInFlight[i] = VK_NULL_HANDLE;

    if(!vulkanSwapchainRecreate(
        &state, 
        state.clientWidth, 
        state.clientHeight))
    {
        PERROR("Failed to recreate swapchain.");
        return false;
    }
    
    for(const auto& cmd : state.commandBuffers){
        vkFreeCommandBuffers(state.device.handle, state.device.commandPool, 1, &cmd.handle);
    }
    state.commandBuffers.clear();

    for(const auto& framebuffer : state.swapchain.framebuffers){
        vkDestroyFramebuffer(state.device.handle, framebuffer.handle, nullptr);
    }

    vulkanRegenerateFramebuffers(
        &state.swapchain,
        &state.pipelines["debug.pipeline"]->renderpass);
    
    createCommandBuffers();
    state.recreatingSwapchain = false;
    return true;
}

void vulkanImguiRender()
{
    imguiRender(state.commandBuffers[state.imageIndex].handle);
}

void vulkanRenderPassCreate(VulkanRenderpass* outRenderpass, const json& j)
{
    // TODO Make configurable from json file ...
    // Colour attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format        = state.swapchain.format.format;
    attachmentDescription.samples       = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachmentDescription.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp= VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription depthAttachmentDescription{};
    depthAttachmentDescription.format           = state.swapchain.depthFormat;
    depthAttachmentDescription.samples          = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDescription.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDescription.finalLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentDescription.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDescription.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference attachmentRef = {};
    attachmentRef.attachment    = 0;
    attachmentRef.layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount     = 1;
    subpassDescription.pColorAttachments        = &attachmentRef;
    subpassDescription.pDepthStencilAttachment  = &depthRef;

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

    VkRenderPassCreateInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount    = 2;
    info.pAttachments       = attachmentDesc;
    info.subpassCount       = 1;
    info.pSubpasses         = &subpassDescription;
    info.dependencyCount    = 2;
    info.pDependencies      = subpassDependencies;

    if(vkCreateRenderPass(state.device.handle, &info, nullptr, &outRenderpass->handle) != VK_SUCCESS)
    {
        PFATAL("Render pass could not be created!");
    }
}

void vulkanRenderPassDestroy(VulkanRenderpass* renderpass)
{
    for(auto r : renderpass->renderTargets)
    {
        vkDestroyFramebuffer(state.device.handle, r.handle, nullptr);
        r.attachments.clear();
    }

    vkDestroyRenderPass(state.device.handle, renderpass->handle, nullptr);
}

bool vulkanCreatePipeline(VulkanPipeline* pipeline, const std::string& name, const json& j)
{
    vulkanRenderPassCreate(&pipeline->renderpass, j);

    // Compile shaders --- One for each stage. At the moment only Vertex and Fragment shader.
    const std::string vsFilename = j.value("vs", "");
    const std::string psFilename = j.value("ps", "");
    // Shader modules creation ---
    std::vector<char> vertexBuffer;
    if(!readShaderFile(compileShader(vsFilename).c_str(), vertexBuffer)) {
        return false;
    }

    std::vector<char> fragBuffer;
    if(!readShaderFile(compileShader(psFilename).c_str(), fragBuffer)) {
        return false;
    }

    vulkanCreateShaderModule(state.device, vertexBuffer, &pipeline->shaderStages[0].shaderModule);
    vulkanCreateShaderModule(state.device, fragBuffer, &pipeline->shaderStages[1].shaderModule);

    // Set the samplers index
    pipeline->samplerUses[0] = TEXTURE_USE_DIFFUSE;
    pipeline->samplerUses[1] = TEXTURE_USE_NORMAL;
    pipeline->samplerUses[2] = TEXTURE_USE_METALLIC_ROUGHNESS;

    // HARDCODED
    pipeline->poolSizes[0] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024};
    pipeline->poolSizes[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096};

    // Create descriptor pool
    VkDescriptorPoolCreateInfo descriptorPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolInfo.poolSizeCount    = 2;
    descriptorPoolInfo.pPoolSizes       = pipeline->poolSizes;
    descriptorPoolInfo.maxSets          = static_cast<u32>(state.swapchain.images.size());
    descriptorPoolInfo.flags            = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(vkCreateDescriptorPool(state.device.handle, &descriptorPoolInfo, nullptr, &pipeline->descriptorPool));

    // Create descriptor set
    pipeline->descriptorSets[0].bindings->binding = 0;  // TODO Make configurable
    pipeline->descriptorSets[0].bindings->descriptorCount = 1;
    pipeline->descriptorSets[0].bindings->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pipeline->descriptorSets[0].bindings->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    //pipeline->descriptorSets[0].bindings->binding = 1;  // TODO Make configurable
    //pipeline->descriptorSets[0].bindings->descriptorCount = 1; // TODO Make configurable
    //pipeline->descriptorSets[0].bindings->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //pipeline->descriptorSets[0].bindings->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // TODO HARDCODED - Only globals at the moment
    pipeline->descriptorSetCount = 1;

    // Create Descriptor set layouts.
    for(u32 i = 0; i < pipeline->descriptorSetCount; ++i)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layoutInfo.bindingCount = 1; //pipeline->descriptorSets[i].bindingCount;
        layoutInfo.pBindings = pipeline->descriptorSets[i].bindings;
        VK_CHECK(vkCreateDescriptorSetLayout(state.device.handle, &layoutInfo, nullptr, &pipeline->descriptorSetLayout[i]));
    }

    // TODO Create shader stages, should be done in separated functions. 
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);

    VkPipelineShaderStageCreateInfo vertexStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertexStageInfo.stage   = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module  = pipeline->shaderStages[0].shaderModule;
    vertexStageInfo.pName   = "main";
    shaderStages.at(0) = (vertexStageInfo);

    VkPipelineShaderStageCreateInfo fragmentStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragmentStageInfo.stage     = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module    = pipeline->shaderStages[1].shaderModule;
    fragmentStageInfo.pName     = "main";
    shaderStages.at(1) = (fragmentStageInfo);

    // Viewport
    VkViewport viewport;
    viewport.x          = 0;
    viewport.y          = state.clientHeight;
    viewport.width      = state.clientWidth;
    viewport.height     = -(f32)state.clientHeight;
    viewport.maxDepth   = 1;
    viewport.minDepth   = 0;

    // Scissors
    VkRect2D scissors;
    scissors.extent = {state.clientWidth, state.clientHeight};
    scissors.offset = {0, 0};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = 0xf;

    const VertexDeclaration* vtx = getVertexDeclarationByName("PosColor");

    vulkanCreateGraphicsPipeline(
        state.device,
        &pipeline->renderpass,
        vtx->size,
        vtx->layout,
        shaderStages.size(),
        shaderStages.data(),
        1,
        pipeline->descriptorSetLayout,
        1,
        &colorBlendAttachment,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        viewport,
        scissors,
        false,
        true,
        pipeline
    );

    pipeline->globalUboStride = sizeof(ViewProjectionBuffer);

    // Uniform buffer // TODO Make dynamic.
    u64 totalSize = sizeof(pipeline->uniformData) + (pipeline->uboStride * VULKAN_MAX_MATERIAL_COUNT);
    if(!vulkanBufferCreate(
        state.device, 
        totalSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &pipeline->uniformBuffer))
    {
        PERROR("Vulkan buffer creation failed on pipeline creation.")
        return false;
    }

    VkDescriptorSetLayout globalLayouts[3] = {
        pipeline->descriptorSetLayout[0],
        pipeline->descriptorSetLayout[0],
        pipeline->descriptorSetLayout[0]
    };

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocInfo.descriptorPool       = pipeline->descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount   = static_cast<u32>(state.swapchain.images.size());
    descriptorSetAllocInfo.pSetLayouts          = globalLayouts;

    // ! This should be moved and created base on pipelines.json json config.
    for(u8 i = 0; i < state.swapchain.imageViews.size(); ++i)
    {
        std::vector<VkImageView> attachments = {state.swapchain.imageViews.at(i), state.swapchain.depthImage.view};
        VulkanRenderTarget* renderTarget = new VulkanRenderTarget();
        vulkanRenderTargetCreate(state.device, attachments, &pipeline->renderpass, state.swapchain.extent.width, state.swapchain.extent.height, renderTarget);
        pipeline->renderpass.renderTargets.emplace_back(*renderTarget);
    }
    
    VK_CHECK(vkAllocateDescriptorSets(state.device.handle, &descriptorSetAllocInfo, pipeline->globalDescriptorSet));
    return true;
}

void vulkanPipelineDestroy(VulkanPipeline* pipeline)
{
    if(!pipeline){
        PERROR("vulkanPipelineDestroy - No pipeline to destroy.");
        return;
    }

    for(u8 i = 0; i < pipeline->descriptorSetCount; ++i){
        vkDestroyDescriptorSetLayout(state.device.handle, pipeline->descriptorSetLayout[i], nullptr);
    }

    if(pipeline->descriptorPool)
        vkDestroyDescriptorPool(state.device.handle, pipeline->descriptorPool, nullptr);

    vulkanBufferDestroy(state.device, pipeline->uniformBuffer);

    vulkanRenderPassDestroy(&pipeline->renderpass);

    if(pipeline->handle)
    {
        vkDestroyPipeline(state.device.handle, pipeline->handle, nullptr);
        pipeline->handle = nullptr;
    }
    if(pipeline->layout)
    {
        vkDestroyPipelineLayout(state.device.handle, pipeline->layout, nullptr);
        pipeline->layout = nullptr;
    }

    for(auto stage : pipeline->shaderStages)
        vkDestroyShaderModule(state.device.handle, stage.shaderModule, nullptr);

}

void vulkanUsePipeline(const std::string& name)
{
    VulkanPipeline* pipeline = state.pipelines[name];
    vulkanBindPipeline(&state.commandBuffers[state.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void vulkanBindPipeline(CommandBuffer* cmd, VkPipelineBindPoint bindPoint, VulkanPipeline* pipeline)
{
    vkCmdBindPipeline(cmd->handle, bindPoint, pipeline->handle);
    state.boundPipeline = pipeline;
}

bool vulkanBindGlobals()
{
    if(state.boundPipeline == nullptr)
    {
        PFATAL("There is no pipeline bound.");
        return false;
    }
    
    state.boundPipeline->boudnUboOffset = 0;
    return true;
}

bool vulkanApplyGlobals()
{
    VulkanPipeline* pipeline = state.boundPipeline;
    u32 imageIdx = state.imageIndex;
    VkCommandBuffer cmdBuffer = state.commandBuffers[imageIdx].handle;
    VkDescriptorSet descriptor = pipeline->globalDescriptorSet[imageIdx];

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = pipeline->uniformBuffer.handle;
    bufferInfo.offset = 0; // TODO Make dynamic from info
    bufferInfo.range = pipeline->globalUboStride;

    VkWriteDescriptorSet uboWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    uboWrite.dstSet = pipeline->globalDescriptorSet[imageIdx];
    uboWrite.dstBinding = 0;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &bufferInfo;

    VkWriteDescriptorSet descriptorWrites[2];
    descriptorWrites[0] = uboWrite;

    vkUpdateDescriptorSets(state.device.handle, 1, descriptorWrites, 0, 0);

    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 1, &descriptor, 0, 0);
    return true;
}

static std::string compileShader(const std::string& name)
{
    std::string path = "./data/shaders/";
    std::string filename = path + name;
    std::string pathCompiled = filename + ".spv";
    std::string compileCmd = "glslc " + filename + " -o " + pathCompiled;
    system(compileCmd.c_str());
    return pathCompiled;
}