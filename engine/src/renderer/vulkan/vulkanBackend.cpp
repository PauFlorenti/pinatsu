#include "vulkanBackend.h"

#include "vulkanTypes.h"
#include "vulkanDevice.h"
#include "vulkanSwapchain.h"
#include "vulkanRenderpass.h"
#include "vulkanFramebuffer.h"
#include "vulkanBuffer.h"
#include "vulkanCommandBuffer.h"
#include "vulkanImage.h"
#include "vulkanUtils.h"
#include "vulkanImgui.h"
#include "vulkanPlatform.h"

#include "shaders/vulkanForwardShader.h"
#include "shaders/vulkanDeferredShader.h"

#include "core/application.h"

//#include "systems/entitySystemComponent.h"
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

    state.forwardShader.globalUboData.view        = cCamera->getView();
    state.forwardShader.globalUboData.projection  = cCamera->getProjection();
    state.forwardShader.globalUboData.position    = cCamera->getEye();

    vulkanBufferLoadData(state.device, state.forwardShader.globalUbo, 0, sizeof(ViewProjectionBuffer), 0, &state.forwardShader.globalUboData);

    TCompLightPoint* lightsAddress = getObjectManager<TCompLightPoint>()->getAddress();
    for(u32 i = 0; i < getObjectManager<TCompLightPoint>()->size(); ++i)
    {
        TCompLightPoint* l = &lightsAddress[i];
        if(l->enabled == false)
            return;

        state.forwardShader.lightData.color     = l->color;
        state.forwardShader.lightData.intensity = l->intensity;
        state.forwardShader.lightData.position  = l->getPosition();
        state.forwardShader.lightData.radius    = l->radius;
        state.forwardShader.lightData.enabled   = l->enabled;
        vulkanBufferLoadData(
            state.device,
            state.forwardShader.lightUbo,
            sizeof(VulkanLightData) * i,
            sizeof(VulkanLightData),
            0,
            &state.forwardShader.lightData);
    }

    vulkanForwardShaderUpdateGlobalData(&state);
}

void
vulkanDeferredUpdateGlobaState(f32 dt)
{
    gameTime += dt;

    // TODO at the moment
    CEntity* hcamera = getEntityByName("camera");
    TCompCamera* cCamera = hcamera->get<TCompCamera>();

    state.deferredShader.globalUboData.view        = cCamera->getView();
    state.deferredShader.globalUboData.projection  = cCamera->getProjection();
    state.deferredShader.globalUboData.position    = cCamera->getEye();

    vulkanBufferLoadData(state.device, state.deferredShader.globalUbo, 0, sizeof(ViewProjectionBuffer), 0, &state.deferredShader.globalUboData);

    TCompLightPoint* lightsAddress = getObjectManager<TCompLightPoint>()->getAddress();
    for(u32 i = 0; i < getObjectManager<TCompLightPoint>()->size(); ++i)
    {
        TCompLightPoint* l = &lightsAddress[i];

        if (l->enabled == false)
            return;

        state.deferredShader.lightData.color = l->color;
        state.deferredShader.lightData.intensity = l->intensity;
        state.deferredShader.lightData.position = l->getPosition();
        state.deferredShader.lightData.radius = l->radius;
        state.deferredShader.lightData.enabled = l->enabled;
        vulkanBufferLoadData(
            state.device,
            state.deferredShader.lightUbo,
            sizeof(VulkanLightData) * i,
            sizeof(VulkanLightData),
            0,
            &state.deferredShader.lightData);
    }

    vulkanDeferredUpdateGlobalData(state.device, state.deferredShader);
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
    return false;
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

    // Render pass
    if(!vulkanRenderPassCreate(&state)){
        return false;
    }

    // Framebuffers
    vulkanRegenerateFramebuffers(
        &state.swapchain, 
        &state.renderpass);

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

    state.vulkanMeshes = (VulkanMesh*)memAllocate(sizeof(VulkanMesh) * VULKAN_MAX_MESHES, MEMORY_TAG_RENDERER);
    for(u32 i = 0; i < VULKAN_MAX_MESHES; ++i) {
        state.vulkanMeshes[i].id = INVALID_ID;
    }

    vulkanCreateForwardShader(&state, &state.forwardShader);
    vulkanDeferredShaderCreate(state.device, state.swapchain, state.swapchain.extent.width, state.swapchain.extent.height, &state.deferredShader);
    imguiInit(&state, &state.renderpass);

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

    PDEBUG("Destroying Vulkan Shaders ...");
    vulkanDestroyForwardShader(&state);
    vulkanDeferredShaderDestroy(state.device, state.deferredShader);

    PDEBUG("Destroying Vulkan Render passes ...");
    vkDestroyRenderPass(state.device.handle, state.renderpass.handle, nullptr);

    for(auto framebuffer : state.swapchain.framebuffers)
    {
        vkDestroyFramebuffer(state.device.handle, framebuffer.handle, nullptr);
    }

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
    vulkanWaitFence(
        state.device, 
        &state.frameInFlightFences[state.currentFrame]);
    vulkanResetFence(state.device, &state.frameInFlightFences[state.currentFrame]);

    // Acquire next image index.
    vkAcquireNextImageKHR(
        state.device.handle, 
        state.swapchain.handle, 
        UINT64_MAX, 
        state.imageAvailableSemaphores.at(state.currentFrame),
        0, 
        &state.imageIndex);

    return true;
}

void
vulkanBeginCommandBuffer(DefaultRenderPasses renderPassid)
{
    // TODO Solve synchronization
    vkDeviceWaitIdle(state.device.handle);
    VkCommandBuffer cmd;
    switch (renderPassid)
    {
    case 0:
        cmd = state.commandBuffers[state.imageIndex].handle;
        break;
    case 1:
        cmd = state.deferredShader.geometryCmdBuffer.handle;
        break;
    case 2:
        cmd = state.commandBuffers[state.imageIndex].handle;
        break;        
    default:
        break;
    }

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
}

bool vulkanBeginRenderPass(DefaultRenderPasses renderPassid)
{
    // TODO Abstract render pass creation.
    switch(renderPassid)
    {
        // Forward render pass
        case 0:
        {
            VkClearValue clearColors[2];
            clearColors[0] = {{0.2f, 0.2f, 0.2f, 1.0f}};
            clearColors[1].depthStencil.depth = 1.0f;
            clearColors[1].depthStencil.stencil = 0;

            VkRenderPassBeginInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            info.renderPass         = state.renderpass.handle;
            info.framebuffer        = state.swapchain.framebuffers.at(state.imageIndex).handle;
            info.renderArea.offset  = {0, 0};
            info.renderArea.extent  = state.swapchain.extent;
            info.clearValueCount    = 2;
            info.pClearValues       = clearColors;
            
            vkCmdBeginRenderPass(state.commandBuffers.at(state.imageIndex).handle, &info, VK_SUBPASS_CONTENTS_INLINE);
            return true;
            break;
        }
        case 1: // Geometry pass
        {
            VkClearValue clearColors[4];
            clearColors[0].color = {0.2, 0.2, 0.2, 1.0};
            clearColors[1].color = {0.2, 0.2, 0.2, 1.0};
            clearColors[2].color = {0.2, 0.2, 0.2, 1.0};
            clearColors[3].depthStencil.depth = 1.0f;
            clearColors[3].depthStencil.stencil = 0;

            VkRenderPassBeginInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            info.renderPass         = state.deferredShader.geometryRenderpass.handle;
            info.framebuffer        = state.deferredShader.geometryFramebuffer.handle;
            info.renderArea.offset  = {0, 0};
            info.renderArea.extent  = state.swapchain.extent;
            info.clearValueCount    = 4;
            info.pClearValues       = clearColors;

            vkCmdBeginRenderPass(state.deferredShader.geometryCmdBuffer.handle, &info, VK_SUBPASS_CONTENTS_INLINE);
            return true;
            break;
        }
        case 2:
        {
            VkClearValue clearColors[2];
            clearColors[0] = {{0.2f, 0.2f, 0.2f, 1.0f}};
            clearColors[1].depthStencil.depth = 1.0f;
            clearColors[1].depthStencil.stencil = 0;

            VkRenderPassBeginInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            info.renderPass         = state.deferredShader.lightRenderpass.handle;
            info.framebuffer        = state.deferredShader.lightFramebuffer[state.imageIndex].handle;
            info.renderArea.offset  = {0, 0};
            info.renderArea.extent  = state.swapchain.extent;
            info.clearValueCount    = 2;
            info.pClearValues       = clearColors;

            vkCmdBeginRenderPass(state.commandBuffers.at(state.imageIndex).handle, &info, VK_SUBPASS_CONTENTS_INLINE);
            return true;
            break;
        }
        default:
            return false;
            break;
    }
}

void vulkanDrawGeometry(DefaultRenderPasses renderPassID, const RenderMeshData* data)
{
    // TODO make material specify the type to render
    Material* m = data->material;
    if(!m){
        Material mat;
        mat.diffuseColor = glm::vec4(1);
        m = &mat;
    }

    VkCommandBuffer cmd;
    switch (renderPassID)
    {
    case 0:
        cmd = state.commandBuffers[state.imageIndex].handle;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state.forwardShader.pipeline.pipeline);
        vulkanForwardShaderSetMaterial(&state, &state.forwardShader, m);
        vkCmdPushConstants(cmd, state.forwardShader.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &data->model);
        break;
    case 1:
        cmd = state.deferredShader.geometryCmdBuffer.handle;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state.deferredShader.geometryPipeline.pipeline);
        vulkanDeferredShaderSetMaterial(&state, &state.deferredShader, m);
        vkCmdPushConstants(cmd, state.deferredShader.geometryPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &data->model);
        break;
    case 2:
        cmd = state.commandBuffers[state.imageIndex].handle;
        vulkanDeferredUpdateGbuffers(state.device, state.imageIndex, state.deferredShader);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state.deferredShader.lightPipeline.pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state.deferredShader.lightPipeline.layout, 0, 1, &state.deferredShader.lightDescriptorSet[state.imageIndex], 0, nullptr);
        break;        
    default:
        break;
    }

    VulkanMesh* geometry = &state.vulkanMeshes[data->mesh->rendererId];

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &geometry->vertexBuffer.handle, &offset);
    //vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    if(geometry->indexCount > 0)
    {
        vkCmdBindIndexBuffer(cmd, geometry->indexBuffer.handle, offset, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, geometry->indexCount, 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(cmd, geometry->vertexCount, 1, 0, 0);
    }
}

void vulkanEndRenderPass(DefaultRenderPasses renderPass)
{
    // TODO end the given render pass
    switch (renderPass)
    {
    case 0:
        vkCmdEndRenderPass(state.commandBuffers[state.imageIndex].handle);
        break;
    case 1:
        vkCmdEndRenderPass(state.deferredShader.geometryCmdBuffer.handle);
        break;
    case 2:
        vkCmdEndRenderPass(state.commandBuffers[state.imageIndex].handle);
        break;
    default:
        break;
    }
}

void
vulkanSubmitCommands(DefaultRenderPasses renderPass)
{
    switch(renderPass)
    {
        case 0:
        {
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

            //vulkanWaitFence(state.device, &state.frameInFlightFences[state.currentFrame]);
            //vulkanResetFence(state.device, &state.frameInFlightFences[state.currentFrame]);

            if(vkQueueSubmit(state.device.graphicsQueue, 1, &submitInfo, state.frameInFlightFences[state.currentFrame].handle) != VK_SUCCESS){
                PERROR("Queue wasn't submitted.");
            }
        }
            break;
        case 1:
        {
            VK_CHECK(vkEndCommandBuffer(state.deferredShader.geometryCmdBuffer.handle));

            VkPipelineStageFlags pipelineStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &state.deferredShader.geometryCmdBuffer.handle;
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = &state.imageAvailableSemaphores[state.currentFrame];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = &state.deferredShader.geometrySemaphore;
            submitInfo.pWaitDstStageMask    = &pipelineStage;

            //vulkanWaitFence(state.device, &state.frameInFlightFences[state.currentFrame]);
            //vulkanResetFence(state.device, &state.frameInFlightFences[state.currentFrame]);
            if(vkQueueSubmit(state.device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS){
                PERROR("Queue wasn't submitted.");
            }
            break;
        }
        case 2:
        {
            VK_CHECK(vkEndCommandBuffer(state.commandBuffers[state.imageIndex].handle));

            VkPipelineStageFlags pipelineStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &state.commandBuffers[state.imageIndex].handle;
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = &state.deferredShader.geometrySemaphore;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = &state.renderFinishedSemaphores[state.currentFrame];
            submitInfo.pWaitDstStageMask    = &pipelineStage;

            if(vkQueueSubmit(state.device.graphicsQueue, 1, &submitInfo, state.frameInFlightFences[state.currentFrame].handle) != VK_SUCCESS){
                PERROR("Queue wasn't submitted.");
            }
        }
            break;
        default:
            break;
    }
}

/**
 * @brief Ends the render passa and command buffers for this frame.
 * Submits info to the graphics queue and presents the image.
 * @param void
 * @return void
 */
void vulkanEndFrame(void)
{
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
 * @param VulkanSwapchain* swapchain
 * @param VulkanRenderpass* renderpass
 * @return void
 */
void vulkanRegenerateFramebuffers(
    VulkanSwapchain* swapchain, 
    VulkanRenderpass* renderpass)
{
    for(u32 i = 0; i < swapchain->imageViews.size(); ++i){

        // TODO Make modular
        std::vector<VkImageView> attachments = {swapchain->imageViews.at(i), swapchain->depthImage.view};

        vulkanFramebufferCreate(
            state.device,
            renderpass,
            state.clientWidth, state.clientHeight,
            static_cast<u32>(attachments.size()),
            attachments,
            &swapchain->framebuffers.at(i)
        );
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
        &state.renderpass);
    
    createCommandBuffers();
    state.recreatingSwapchain = false;
    return true;
}

void vulkanImguiRender(const RenderPacket& packet)
{
    imguiRender(state.commandBuffers[state.imageIndex].handle, packet);
}