#include "vulkanBackend.h"
#include "vulkanTypes.h"
#include "vulkanDevice.h"
#include "vulkanSwapchain.h"
#include "vulkanRenderpass.h"
#include "vulkanFramebuffer.h"

#include "core\application.h"
#include "platform\platform.h"
#include "scene/scene.h"
#include "pmath.h"
#include <vector>
#include <string>
#include <fstream>

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

/**
 * Shader Object Create
 * TODO Break it into functions
 */

bool vulkanShaderObjectCreate(VulkanState* pState);

void createCommandBuffers();

/**
 * Synchronization Utility Functions.
 * This includes both Semaphores and Fences.
 */
internal bool vulkanCreateFence(
    VulkanState* pState,
    VulkanFence* fence,
    bool signaled)
{
    VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if(signaled){
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        fence->signaled = true;
    } else {
        fence->signaled = false;
    }

    if(vkCreateFence(state.device.handle, &info, nullptr, &fence->handle) != VK_SUCCESS){
        PERROR("Fence creation failed!");
        return false;
    }
    return true;
};

internal bool vulkanWaitFence(
    VulkanState *pState,
    VulkanFence* fence,
    u64 timeout = UINT64_MAX)
{
    if(fence->signaled)
        return true;
    
    if(vkWaitForFences(
        pState->device.handle, 1, 
        &fence->handle, VK_TRUE, timeout) == VK_SUCCESS)
    {
        return true;
    }
    return false;
}

internal bool vulkanResetFence(
    VulkanState* pState,
    VulkanFence* fence)
{
    if(vkResetFences(
        pState->device.handle, 
        1, 
        &fence->handle) == VK_SUCCESS)
    {
        fence->signaled = false;
        return true;   
    }
    return false;
}

internal void vulkanDestroyFence(
    VulkanState& state,
    VulkanFence& fence)
{
    vkDestroyFence(
        state.device.handle,
        fence.handle,
        nullptr);
}

internal bool vulkanCreateSemaphore(
    VulkanState* pState,
    VkSemaphore* semaphore)
{
    VkSemaphoreCreateInfo info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if(vkCreateSemaphore(pState->device.handle, &info, nullptr, semaphore) != VK_SUCCESS){
        PERROR("Semaphore creation failed!");
        return false;
    }
    return true;
}

internal void vulkanDestroySemaphore(
    VulkanState& state,
    VkSemaphore& semaphore)
{
    vkDestroySemaphore(
        state.device.handle,
        semaphore,
        nullptr);
}

/**
 * Framebuffers functions
 */
void vulkanRegenerateFramebuffers(
    VulkanSwapchain* swapchain, 
    VulkanRenderpass* renderpass);

bool recreateSwapchain();

void vulkanCreateShaderModule(
    VulkanState* pState,
    std::vector<char>& buffer,
    VkShaderModule* module);

void vulkanDestroyShaderModule(
    VulkanState& state,
    ShaderObject& module);

/**
 * *Buffer functions
 */
bool vulkanBufferCreate(
    VulkanState* pState,
    u32 size,
    u32 usageFlags,
    u32 memFlagss,
    VulkanBuffer* buffer);

void vulkanBufferDestroy(
    VulkanState& pState,
    VulkanBuffer& buffer);

void vulkanTransferBuffer(
    VkBuffer &src,
    VkBuffer &dst,
    VkDeviceSize size);

internal void vulkanBufferLoadData(
    VulkanState* pState,
    VulkanBuffer* buffer,
    VkDeviceSize offset,
    u64 size,
    VkMemoryMapFlags flags,
    const void* data);

internal void vulkanUploadDataToGPU(
    VulkanBuffer& buffer,
    u32 offset,
    u64 size,
    const void* data);

internal void vulkanBufferCopyToImage(
    VulkanState* pState,
    VulkanBuffer* buffer,
    VulkanImage* image,
    u32 width,
    u32 height);

//
// * Texture functions
//
internal void vulkanCreateImage(
    VulkanState* pState,
    VkImageType type,
    u32 width,
    u32 height,
    VkFormat imageFormat,
    VkImageTiling tiling,
    VkImageUsageFlags,
    VkMemoryPropertyFlags,
    bool createView,
    VkImageAspectFlags viewAspectFlags,
    VulkanImage* outImage);

internal void vulkanCreateImageView(
    VulkanState* pState, 
    VulkanImage* image,
    VkFormat format,
    VkImageAspectFlags aspectFlags);

internal void vulkanImageTransitionLayout(
    VulkanState* pState,
    VulkanImage* image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkCommandBuffer& cmd);

internal void vulkanCreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding binding{};
    binding.binding         = 0;
    binding.descriptorCount = 1;
    binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding entityBinding{};
    entityBinding.binding           = 1;
    entityBinding.descriptorCount   = 1;
    entityBinding.descriptorType    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    entityBinding.stageFlags        = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.bindingCount   = 1;
    info.pBindings      = &binding;

    VK_CHECK(vkCreateDescriptorSetLayout(state.device.handle, &info, nullptr, &state.descriptorLayout));
}

// Get standard attribute description.
std::vector<VkVertexInputAttributeDescription>
getStandardAttributeDescription(void);

/**
 * * Command buffer functions
 */
// TODO Command buffer functions
//void vulkanCommandBufferFree(VulkanState* pState, VkCommandPool pool, )

void vulkanCommandBufferAllocateAndBeginSingleUse(
    VulkanState* pState,
    VkCommandPool pool,
    VkCommandBuffer& cmd);

void vulkanCommandBufferEndSingleUse(
    VulkanState* pState,
    VkCommandPool pool,
    VkCommandBuffer& cmd);

i32 findMemoryIndex(u32 typeFilter, VkMemoryPropertyFlags memFlags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(state.device.physicalDevice, &memProperties);
    for(u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if(typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & memFlags) == memFlags){
            return i;
        }
    }
    return -1;
}

void vulkanUpdateGlobalState(const glm::mat4 view, const glm::mat4 projection, f32 dt)
{
    gameTime += dt;
    f32 speed = 100.0f;
    state.ubo.view        = view;
    state.ubo.projection  = projection;

    void* data;
    u32 index = (state.currentFrame + 1) % state.swapchain.imageCount;
    vulkanBufferLoadData(&state, &state.uniformBuffers.at(index), 0, sizeof(ViewProjectionBuffer), 0, &state.ubo);
}

bool vulkanCreateMesh(Mesh* mesh, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices)
{
    if(!vertexCount || !vertices) {
        PERROR("vulkanCreateMesh - No vertices available. Unable to create mesh.");
        return false;
    }

    VulkanMesh* auxiliarMesh = (VulkanMesh*)memAllocate(sizeof(VulkanMesh), MEMORY_TAG_ENTITY);

    for(u32 i = 0; i < VULKAN_MAX_MESHES; ++i) {
        if(state.vulkanMeshes[i].id == INVALID_ID){
            mesh->rendererId = i;
            state.vulkanMeshes[i].id = i;
            auxiliarMesh = &state.vulkanMeshes[i];
            break;
        }
    }

    u64 size = vertexCount * sizeof(VulkanVertex);

    auxiliarMesh->vertexCount   = vertexCount;
    auxiliarMesh->vertexOffset  = 0;
    auxiliarMesh->vertexSize    = size;

    // TODO indices

    u32 flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vulkanBufferCreate(&state, size, flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &auxiliarMesh->buffer);

    u32 offset = 0;
    vulkanUploadDataToGPU(auxiliarMesh->buffer, offset, size, vertices);

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
            vulkanBufferDestroy(state, state.vulkanMeshes[i].buffer);
            state.vulkanMeshes[i].id = INVALID_ID;
            break;
        }
    }
}

bool vulkanCreateTexture(void* data, Texture* texture)
{
    if(!data || !texture) {
        PERROR("vulkanCreateTexture - Unable to create the texture given the inputs.");
        return false;
    }

    //VulkanTexture* texture = (VulkanTexture*)texture->data;

    VkDeviceSize textureSize = texture->width * texture->height * texture->channels;

    // ! Assume 8 bit per channel
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    // Staging buffer, load data into it.
    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memPropsFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging;
    vulkanBufferCreate(&state, textureSize, usageFlags, memPropsFlags, &staging);

    vulkanBufferLoadData(&state, &staging, 0, textureSize, 0, texture->data);

    vulkanCreateImage(
        &state,
        VK_IMAGE_TYPE_2D,
        texture->width,
        texture->height,
        format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &state.texture.image
    );

    VkCommandBuffer temporalCommand;
    vulkanCommandBufferAllocateAndBeginSingleUse(&state, state.device.commandPool, temporalCommand);

    vulkanImageTransitionLayout(
        &state, &state.texture.image, 
        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, temporalCommand);

    vulkanBufferCopyToImage(
        &state,
        &staging, 
        &state.texture.image, 
        state.texture.image.width, 
        state.texture.image.height);
    
    vulkanImageTransitionLayout(
        &state, &state.texture.image, 
        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, temporalCommand);

    vulkanCommandBufferEndSingleUse(&state, state.device.commandPool, temporalCommand);

    vulkanBufferDestroy(state, staging);

    return true;
}

void vulkanDestroyTexture(const Texture* texture)
{
    // TODO destroy specific texture
}

/**
 * @brief Initialize all vulkan render system.
 * @param const char* application name.
 * @return bool if succeded initialization.
 */
bool vulkanBackendInit(const char* appName)
{
    applicationGetFramebufferSize(&state.clientWidth, &state.clientHeight);

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
        if(!vulkanCreateSemaphore(&state, &state.imageAvailableSemaphores.at(i))){
            return false;
        }
        if(!vulkanCreateSemaphore(&state, &state.renderFinishedSemaphores.at(i))){
            return false;
        }
        if(!vulkanCreateFence(&state, &state.frameInFlightFences.at(i), true)){
            return false;
        }
    }

    state.vulkanMeshes = (VulkanMesh*)memAllocate(sizeof(VulkanMesh) * VULKAN_MAX_MESHES, MEMORY_TAG_RENDERER);
    for(u32 i = 0; i < VULKAN_MAX_MESHES; ++i) {
        state.vulkanMeshes[i].id = INVALID_ID;
    }

    // Creating ubo for each frame since some frames may still be in flight
    // when we want to upload new data.
    for(size_t i = 0; i < state.swapchain.images.size(); i++)
    {
        VulkanBuffer buffer;

        vulkanBufferCreate(
            &state,
            sizeof(ViewProjectionBuffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &buffer);
        
        state.uniformBuffers.push_back(buffer);
    }

    // Create descriptor pool
    VkDescriptorPoolSize descriptorPoolSize{};
    descriptorPoolSize.descriptorCount = static_cast<u32>(state.swapchain.images.size());
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolCreateInfo descriptorPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = &descriptorPoolSize;
    descriptorPoolInfo.maxSets = static_cast<u32>(state.swapchain.images.size());

    VK_CHECK(vkCreateDescriptorPool(state.device.handle, &descriptorPoolInfo, nullptr, &state.descriptorPool));

    // Create descriptor set layout
    vulkanCreateDescriptorSetLayout();
    std::vector<VkDescriptorSetLayout> layouts(state.swapchain.images.size(), state.descriptorLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocInfo.descriptorPool       = state.descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount   = static_cast<u32>(state.swapchain.images.size());
    descriptorSetAllocInfo.pSetLayouts          = layouts.data();
    
    state.descriptorSet.resize(state.swapchain.images.size());
    VK_CHECK(vkAllocateDescriptorSets(state.device.handle, &descriptorSetAllocInfo, state.descriptorSet.data()));

    for(size_t i = 0; i < state.swapchain.images.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer   = state.uniformBuffers.at(i).handle;
        bufferInfo.offset   = 0;
        bufferInfo.range    = sizeof(ViewProjectionBuffer);

        VkWriteDescriptorSet descriptorWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptorWrite.dstSet              = state.descriptorSet.at(i);
        descriptorWrite.dstBinding          = 0;
        descriptorWrite.dstArrayElement     = 0;
        descriptorWrite.descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount     = 1;
        descriptorWrite.pBufferInfo         = &bufferInfo;
        descriptorWrite.pImageInfo          = nullptr;
        descriptorWrite.pTexelBufferView    = nullptr;

        vkUpdateDescriptorSets(state.device.handle, 1, &descriptorWrite, 0, nullptr);
    }

    // Shaders Modules and main pipeline
    if(!vulkanShaderObjectCreate(&state)){
        return false;
    }

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
        vulkanDestroySemaphore(state, semaphore);
    }
    for(VkSemaphore& semaphore : state.renderFinishedSemaphores)
    {
        vulkanDestroySemaphore(state, semaphore);
    }
    for(VulkanFence& fence : state.frameInFlightFences)
    {
        vulkanDestroyFence(state, fence);
    }

    // Destroy all buffers from loaded meshes
    for(u32 i = 0;
        i < VULKAN_MAX_MESHES;
        ++i)
    {
        if(state.vulkanMeshes[i].id != INVALID_ID)
        {
            vulkanBufferDestroy(state, state.vulkanMeshes[i].buffer);
        }
    }

    // Destroy all images.
    vkDestroyImage(state.device.handle, state.texture.image.handle, nullptr);
    vkDestroyImageView(state.device.handle, state.texture.image.view, nullptr);

    for(size_t i = 0; i < state.swapchain.images.size(); i++)
    {
        vulkanBufferDestroy(state, state.uniformBuffers.at(i));
    }

    vulkanDestroyShaderModule(state, state.vertexShaderObject);
    vulkanDestroyShaderModule(state, state.fragmentShaderObject);

    vkDestroyDescriptorSetLayout(state.device.handle, state.descriptorLayout, nullptr);
    vkDestroyDescriptorPool(state.device.handle, state.descriptorPool, nullptr);

    vkDestroyPipelineLayout(state.device.handle, state.graphicsPipeline.layout, nullptr);

    vkDestroyPipeline(state.device.handle, state.graphicsPipeline.pipeline, nullptr);

    vkDestroyRenderPass(state.device.handle, state.renderpass.handle, nullptr);

    for(auto framebuffer : state.swapchain.framebuffers)
    {
        vkDestroyFramebuffer(state.device.handle, framebuffer.handle, nullptr);
    }
    vulkanSwapchainDestroy(&state);


    vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
    destroyLogicalDevice(state);

    // Debug Messenger
#ifdef DEBUG
    vulkanDestroyDebugMessenger(state);
#endif

    vkDestroyInstance(state.instance, nullptr);
}

/**
 * @brief Do necessary preparation to begin the frame rendering.
 * If failed, vulkanDraw won't be called and rendering pass won't perform.
 * @param void
 * @return bool if succeded.
 */

static bool done = false;

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
        &state, 
        &state.frameInFlightFences[state.currentFrame]);

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

    VkViewport viewport;
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = state.clientWidth;
    viewport.height     = state.clientHeight;
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor;
    scissor.extent.width    = state.clientWidth;
    scissor.extent.height   = state.clientHeight;
    scissor.offset.x        = 0.0;
    scissor.offset.y        = 0.0;

    vkCmdSetViewport(state.commandBuffers.at(state.imageIndex).handle, 0, 1, &viewport);
    vkCmdSetScissor(state.commandBuffers.at(state.imageIndex).handle, 0, 1, &scissor);

    VkClearValue clearColor = {{0.2f, 0.2f, 0.2f, 1.0f}};

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

/**
 * @brief Perform the rendering action. Binds a pipeline and draws.
 * @param void
 * @return bool
 */
bool vulkanDraw(const Scene& scene)
{
    vkCmdBindPipeline(state.commandBuffers[state.imageIndex].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, state.graphicsPipeline.pipeline);

    for(const Entity& ent : scene.entities)
    {
        VulkanMesh* mesh = &state.vulkanMeshes[ent.mesh->rendererId];
        VkDeviceSize offset = {0};
        vkCmdBindVertexBuffers(state.commandBuffers[state.imageIndex].handle, 0, 1, &mesh->buffer.handle, &offset);
        vkCmdBindDescriptorSets(state.commandBuffers[state.imageIndex].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, state.graphicsPipeline.layout,
            0, 1, &state.descriptorSet[state.imageIndex], 0, nullptr);
        vkCmdPushConstants(state.commandBuffers[state.imageIndex].handle, state.graphicsPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &ent.model);
        u32 vCount = state.vulkanMeshes[ent.mesh->rendererId].vertexCount;
        vkCmdDraw(state.commandBuffers[state.imageIndex].handle, vCount, 1, 0, 0);
    }
    return true;
}

/**
 * @brief Ends the render passa and command buffers for this frame.
 * Submits info to the graphics queue and presents the image.
 * @param void
 * @return void
 */
void vulkanEndFrame(void)
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

    vulkanWaitFence(&state, &state.frameInFlightFences[state.currentFrame]);
    vulkanResetFence(&state, &state.frameInFlightFences[state.currentFrame]);

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

// TODO create a function to read files and treat shaders accordingly.
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

// TODO Separate shader and pipeline creation.
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

    vulkanCreateShaderModule(&state, vertexBuffer, &pState->vertexShaderObject.shaderModule);
    vulkanCreateShaderModule(&state, fragBuffer, &pState->fragmentShaderObject.shaderModule);

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
    VkVertexInputBindingDescription vertexBinding = {};
    vertexBinding.binding   = 0;
    vertexBinding.stride    = sizeof(f32) * 7;
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> inputAttributes = getStandardAttributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo   = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputStateInfo.vertexAttributeDescriptionCount        = inputAttributes.size();
    vertexInputStateInfo.pVertexAttributeDescriptions           = inputAttributes.data();
    vertexInputStateInfo.vertexBindingDescriptionCount          = 1;
    vertexInputStateInfo.pVertexBindingDescriptions             = &vertexBinding;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyInfo.topology                  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable    = VK_FALSE;

    VkViewport viewport = {};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f; //pState->clientHeight;
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

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicStateInfo.dynamicStateCount  = static_cast<u32>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates     = dynamicStates.data();

    // TODO Implement push constants for entity model matrix.
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size          = sizeof(mat4);
    pushConstantRange.offset        = 0;
    pushConstantRange.stageFlags    = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutInfo.pushConstantRangeCount   = 1;
    layoutInfo.pPushConstantRanges      = &pushConstantRange;
    layoutInfo.setLayoutCount           = 1;
    layoutInfo.pSetLayouts              = &state.descriptorLayout;

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
    info.pDynamicState          = &dynamicStateInfo;
    info.pTessellationState     = nullptr;
    info.layout                 = pState->graphicsPipeline.layout;
    info.renderPass             = pState->renderpass.handle;
    info.subpass                = 0;

    VK_CHECK(vkCreateGraphicsPipelines(pState->device.handle, VK_NULL_HANDLE, 1, &info, nullptr, &pState->graphicsPipeline.pipeline));

    return true;
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
        std::vector<VkImageView> attachments = {swapchain->imageViews.at(i)};

        vulkanFramebufferCreate(
            &state,
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

    //TODO is this necessary?
    // Regenerate the ubo buffers.
    for(size_t i = 0; i < state.uniformBuffers.size(); i++)
    {
        vulkanBufferDestroy(state, state.uniformBuffers.at(i));
    }
    state.uniformBuffers.clear();

    // Destroy and regenerate the descriptor pool
    vkDestroyDescriptorPool(state.device.handle, state.descriptorPool, nullptr);

    VkDescriptorPoolSize descriptorPoolSize{};
    descriptorPoolSize.descriptorCount  = static_cast<u32>(state.swapchain.images.size());
    descriptorPoolSize.type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolCreateInfo descriptorPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolInfo.poolSizeCount    = 1;
    descriptorPoolInfo.pPoolSizes       = &descriptorPoolSize;
    descriptorPoolInfo.maxSets          = static_cast<u32>(state.swapchain.images.size());

    VK_CHECK(vkCreateDescriptorPool(state.device.handle, &descriptorPoolInfo, nullptr, &state.descriptorPool));

    vkDestroyDescriptorSetLayout(state.device.handle, state.descriptorLayout, nullptr);
    vulkanCreateDescriptorSetLayout();
    std::vector<VkDescriptorSetLayout> layouts(state.swapchain.images.size(), state.descriptorLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocInfo.descriptorPool       = state.descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount   = static_cast<u32>(state.swapchain.images.size());
    descriptorSetAllocInfo.pSetLayouts          = layouts.data();
    
    state.descriptorSet.resize(state.swapchain.images.size());
    VK_CHECK(vkAllocateDescriptorSets(state.device.handle, &descriptorSetAllocInfo, state.descriptorSet.data()));

    // Recreate each uniform buffer.
    // TODO is this necessary?
    for(size_t i = 0; i < state.swapchain.images.size(); i++)
    {
        VulkanBuffer buffer;

        vulkanBufferCreate(
            &state,
            sizeof(ViewProjectionBuffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &buffer);
        
        state.uniformBuffers.push_back(buffer);
    }

    // Re-Update each descriptor set.
    // TODO is this necessary?
    for(size_t i = 0; i < state.swapchain.images.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer   = state.uniformBuffers.at(i).handle;
        bufferInfo.offset   = 0;
        bufferInfo.range    = sizeof(ViewProjectionBuffer);

        VkWriteDescriptorSet descriptorWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptorWrite.dstSet              = state.descriptorSet.at(i);
        descriptorWrite.dstBinding          = 0;
        descriptorWrite.dstArrayElement     = 0;
        descriptorWrite.descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount     = 1;
        descriptorWrite.pBufferInfo         = &bufferInfo;
        descriptorWrite.pImageInfo          = nullptr;
        descriptorWrite.pTexelBufferView    = nullptr;

        vkUpdateDescriptorSets(state.device.handle, 1, &descriptorWrite, 0, nullptr);
    }

    // TODO Change viewport ans scissor from render pipeline

    state.recreatingSwapchain = false;
    return true;
}

/**
 * @brief Creates a shader module given a valid buffer with the
 * shader binary data in it.
 * @param VulkanState* pState,
 * @param std::vector<char>& buffer conaining all spvr binary data
 * @param VkShaderModule* The shader module to be created.
 * @return void
 */
void vulkanCreateShaderModule(
    VulkanState* pState,
    std::vector<char>& buffer,
    VkShaderModule* module)
{
    VkShaderModuleCreateInfo info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize   = static_cast<u32>(buffer.size());
    info.pCode      = reinterpret_cast<const u32*>(buffer.data());
    
    VK_CHECK(vkCreateShaderModule(
        pState->device.handle, 
        &info, 
        nullptr, 
        module));
}

/**
 * TODO Revise if it should be Destroy ShaderObject
 */
void vulkanDestroyShaderModule(
    VulkanState& state,
    ShaderObject& module)
{
    vkDestroyShaderModule(
        state.device.handle,
        module.shaderModule,
        nullptr);
}

// ******************
// * Buffer functions
// ******************

bool vulkanBufferCreate(
    VulkanState* pState,
    u32 size,
    u32 usageFlags,
    u32 memFlags,
    VulkanBuffer* buffer)
{
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size                   = size;
    info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE; // It is only used in one queue
    info.usage                  = usageFlags;

    VK_CHECK(vkCreateBuffer(pState->device.handle, &info, nullptr, &buffer->handle));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(pState->device.handle, buffer->handle, &requirements);

    i32 index = findMemoryIndex(requirements.memoryTypeBits, memFlags);
    if(index == -1){
        PERROR("Could not find a valid memory index.");
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize    = requirements.size;
    allocInfo.memoryTypeIndex   = index;

    vkAllocateMemory(pState->device.handle, &allocInfo, nullptr, &buffer->memory);
    vkBindBufferMemory(pState->device.handle, buffer->handle, buffer->memory, 0);

    return true;
}

/**
 * Upload data to the given buffer by mapping it.
 */
internal void vulkanBufferLoadData(
    VulkanState* pState,
    VulkanBuffer* buffer,
    VkDeviceSize offset,
    u64 size,
    VkMemoryMapFlags flags,
    const void* data)
{
    void* targetData;
    vkMapMemory(pState->device.handle, buffer->memory, offset, size, flags, &targetData);
    std::memcpy(targetData, data, size);
    vkUnmapMemory(pState->device.handle, buffer->memory);
}

/**
 * @brief Receives a buffer to destroy
 * @param VulkanState& pState
 * @param VkBuffer& buffer to destroy
 * @return void
 */
void vulkanBufferDestroy(
    VulkanState& pState,
    VulkanBuffer& buffer
)
{
    vkFreeMemory(
        pState.device.handle, 
        buffer.memory, 
        nullptr);

    vkDestroyBuffer(
        pState.device.handle, 
        buffer.handle,
        nullptr);
}

/**
 * @brief Transfer the buffer info from source buffer to destination buffer.
 * @param VkBuffer src The source of the data to be transferred.
 * @param VkBuffer dst The destination of the data.
 * @param VkDeviceSize size of the data to be transferred.
 * @return void
 */
void vulkanTransferBuffer(
    VkBuffer& src,
    VkBuffer& dst,
    VkDeviceSize size
)
{
    // TODO may be useful to create a specific command pool for such commands.
    VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = state.device.transferCmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(state.device.handle, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &beginInfo);

    VkBufferCopy region;
    region.srcOffset    = 0;
    region.dstOffset    = 0;
    region.size         = size;
    vkCmdCopyBuffer(cmd, src, dst, 1, &region);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    // We want the app to wait until transfer is completed.
    vkQueueSubmit(state.device.transferQueue, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(state.device.transferQueue);

    vkFreeCommandBuffers(state.device.handle, state.device.transferCmdPool, 1, &cmd);
}

/**
 * @brief Upload data to the GPU through a staging buffer.
 */
internal void vulkanUploadDataToGPU(
    VulkanBuffer& buffer, 
    u32 offset, 
    u64 size, 
    const void* data)
{
    VulkanBuffer stagingBuffer;
    
    vulkanBufferCreate(
        &state, 
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer
    );

    vulkanBufferLoadData(&state, &stagingBuffer, 0, size, 0, data);
    vulkanTransferBuffer(stagingBuffer.handle, buffer.handle, size);
    vulkanBufferDestroy(state, stagingBuffer);
}

/**
 * @brief Copy the buffer data to an image.
 */
internal void vulkanBufferCopyToImage(
    VulkanState* pState,
    VulkanBuffer* buffer,
    VulkanImage* image,
    u32 width,
    u32 height)
{
    VkCommandBuffer temporalCommandBuffer;
    vulkanCommandBufferAllocateAndBeginSingleUse(
        pState, 
        pState->device.commandPool, 
        temporalCommandBuffer);
    
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.mipLevel = 0;

    vkCmdCopyBufferToImage(
        temporalCommandBuffer, 
        buffer->handle, 
        image->handle, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        1, 
        &region);

    vulkanCommandBufferEndSingleUse(
        pState,
        pState->device.commandPool,
        temporalCommandBuffer);
}

/**
 * Function to create an image in vulkan.
 */
internal void vulkanCreateImage(
    VulkanState* pState,
    VkImageType type,
    u32 width,
    u32 height,
    VkFormat imageFormat,
    VkImageTiling tiling,
    VkImageUsageFlags imageUsage,
    VkMemoryPropertyFlags memoryProperties,
    bool createView,
    VkImageAspectFlags viewAspectFlags,
    VulkanImage* outImage)
{

    outImage->width = width;
    outImage->height = height;

    VkImageCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType = type;
    info.format = imageFormat;
    info.extent = {width, height, 1};
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.tiling = tiling;
    info.usage = imageUsage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    //info.queueFamilyIndexCount = 1;
    //info.pQueueFamilyIndices = &pState->device.graphicsQueueIndex; ignored as it is not VK_SHARING_MODE_CONCURRENT
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(pState->device.handle, &info, nullptr, &outImage->handle));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(
        pState->device.handle, 
        outImage->handle, 
        &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryIndex(memoryRequirements.memoryTypeBits, memoryProperties);

    VK_CHECK(vkAllocateMemory(pState->device.handle, &memoryAllocateInfo, nullptr, &outImage->memory));

    VK_CHECK(vkBindImageMemory(pState->device.handle, outImage->handle, outImage->memory, 0));

    if(createView)
    {
        outImage->view = 0;
        vulkanCreateImageView(
            pState, 
            outImage, 
            VK_FORMAT_R8G8B8A8_UNORM, 
            viewAspectFlags);
    }
}

internal void vulkanCreateImageView(
    VulkanState* pState, 
    VulkanImage* image,
    VkFormat format,
    VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo info {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    info.image = image->handle;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;  // TODO make configurable
    info.format = format;
    info.subresourceRange.aspectMask = aspectFlags;

    //TODO make configurable
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.baseArrayLayer = 0;

    VK_CHECK(vkCreateImageView(pState->device.handle, &info, nullptr, &image->view));
}

internal void vulkanImageTransitionLayout(
    VulkanState* pState,
    VulkanImage* image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkCommandBuffer& cmd)
{
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.image = image->handle;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = pState->device.graphicsQueueIndex;
    barrier.dstQueueFamilyIndex = pState->device.graphicsQueueIndex;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

// ****************************
// * Command buffer functions *
// ****************************
void vulkanCommandBufferAllocateAndBeginSingleUse(
    VulkanState* pState,
    VkCommandPool pool,
    VkCommandBuffer& cmd)
{
    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(pState->device.handle, &allocInfo, &cmd));

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
}

void vulkanCommandBufferEndSingleUse(
    VulkanState* pState,
    VkCommandPool pool,
    VkCommandBuffer& cmd)
{
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VK_CHECK(vkQueueSubmit(pState->device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

    VK_CHECK(vkQueueWaitIdle(pState->device.graphicsQueue));

    vkFreeCommandBuffers(pState->device.handle, pool, 1, &cmd);
}

/**
 * @brief returns a vector containing the standard input attribute
 * description for this engine. The attributes description is as follows:
 *  vec3 position
 *  vec4 colour
 *  vec3 normal
 *  vec2 uvs
 * @param void
 * @return vector containing all input attributes.
 */
// TODO Add Normals and uvs
std::vector<VkVertexInputAttributeDescription>
    getStandardAttributeDescription(void)
{
    std::vector<VkVertexInputAttributeDescription> attributes(2);
    
    // Position
    VkVertexInputAttributeDescription vert{};
    vert.binding    = 0;
    vert.location   = 0;
    vert.format     = VK_FORMAT_R32G32B32_SFLOAT;
    vert.offset     = 0;
    attributes.at(0) = vert;

    VkVertexInputAttributeDescription color{};
    color.binding   = 0;
    color.location  = 1;
    color.format    = VK_FORMAT_R32G32B32A32_SFLOAT;
    color.offset    = sizeof(f32) * 3;
    attributes.at(1) = color;

    return attributes;
}
