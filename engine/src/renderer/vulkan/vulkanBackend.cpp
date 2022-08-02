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
 * @brief Vulkan Debug Messenger Functions
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

void vulkanLoadRenderpasses()
{
    // Take the json holding all renderpasses configurations.
    json j = loadJson("data/renderpasses.json");

    // Create each render pass.
    for(auto jpass : j.items())
    {
        const std::string& name = jpass.key() + ".renderpass";
        const json& jdef = jpass.value();

        VulkanRenderpass* renderpass = nullptr;
        auto it = state.renderpasses.find(name.c_str());
        if(it != state.renderpasses.end())
        {
            renderpass = it->second;
            PWARN("Render pass %s already loaded in system.");
        }
        else
        {
            renderpass = new VulkanRenderpass();
            state.renderpasses[name] = renderpass;
        }

        vulkanCreateRenderPass(renderpass, jdef);
    }
}

void vulkanLoadPipelines()
{
    // Take the json holding all pipelines configurations.
    json j = loadJson("data/pipelines.json");

    // Create each render pipeline.
    for(auto jpipeline : j.items())
    {
        const std::string& name = jpipeline.key() + ".pipeline";
        const json& jdef = jpipeline.value();

        VulkanRenderPipeline* pipeline = nullptr;
        auto it = state.pipelines.find(name.c_str());
        if(it != state.pipelines.end())
        {
            pipeline = it->second;
            PWARN("Render pipeline %s already loaded in system.");
        }
        else
        {
            pipeline = new VulkanRenderPipeline();
            pipeline->pipelineConfig.name = name;
            state.pipelines[name] = pipeline;
        }

        if(vulkanCreateRenderPipeline(pipeline, jdef) == false)
            PERROR("Failed to create render pipeline %s", name);
    }
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

bool vulkanCreateMaterial(const std::string& pipelineName, Material* m)
{

    VulkanRenderPipeline* pipeline = state.pipelines[pipelineName];

    u32 index = INVALID_ID;
    for(u32 i = 0; i < VULKAN_MAX_MATERIAL_COUNT; ++i)
    {
        if(pipeline->materialInstances[i].id == INVALID_ID)
        {
            pipeline->materialInstances[i].id = i;
            m->rendererId = i;
            index = i;
            break;
        }
    }

    if(index == INVALID_ID)
    {
        PERROR("VulkanCreateMaterial - failed to acquire a valid id for material '%s'.", m->name)
        return false;
    }

    VulkanMaterialInstance* instance = &pipeline->materialInstances[index];

    for(u32 i = 0; i < 1; ++i)
    {
        for(u32 j = 0; j < 3; j++)
        {
            instance->descriptorState[i].generations[j] = INVALID_ID;
            instance->descriptorState[i].ids[j] = INVALID_ID;
        }
    }

    VkDescriptorSetLayout layouts[3] = {
        pipeline->descriptorSetLayout[1],
        pipeline->descriptorSetLayout[1],
        pipeline->descriptorSetLayout[1]
    };

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool        = pipeline->descriptorPool;
    allocInfo.descriptorSetCount    = 3;
    allocInfo.pSetLayouts           = layouts;
    VkResult result = vkAllocateDescriptorSets(
        state.device.handle, 
        &allocInfo, 
        instance->descriptorSets);
    
    if(result != VK_SUCCESS){
        PERROR("VulkanCreateMaterial - Could not allocate descriptor sets for material instance in pipeline '%s'.", pipeline->pipelineConfig.name.c_str());
        return false;
    }
        
    return true;
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

    vulkanLoadRenderpasses();

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

    imguiInit(&state, state.pipelines["debug.pipeline"]->renderpass);

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
    for(std::map<std::string, VulkanRenderPipeline*>::iterator it = state.pipelines.begin(); it != state.pipelines.end(); ++it)
        vulkanDestroyRenderPipeline(it->second);

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
    VulkanRenderpass* renderPass = state.pipelines[renderpass]->renderpass; //state.renderpasses[renderpass];

    VkRenderPassBeginInfo renderpassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderpassInfo.renderPass = renderPass->handle;
    renderpassInfo.framebuffer = renderPass->renderTargets[state.imageIndex].handle;
    // TODO This should be given by the renderpas.
    renderpassInfo.renderArea.offset = {0, 0};
    renderpassInfo.renderArea.extent = state.swapchain.extent;

    // TODO Should be driven by the renderpass info.
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

    // TODO Temps set material ...

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd.handle, 0, 1, &geometry->vertexBuffer.handle, &offset);
    //vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
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

    vulkanRegenerateFramebuffers(
        &state.swapchain,
        state.pipelines["debug.pipeline"]->renderpass);
    
    createCommandBuffers();
    state.recreatingSwapchain = false;
    return true;
}

void vulkanImguiRender()
{
    imguiRender(state.commandBuffers[state.imageIndex].handle);
}

void vulkanCreateRenderPass(VulkanRenderpass* outRenderpass, const json& j)
{
    outRenderpass->clearColor = loadVec4(j, "color");
    outRenderpass->previousPass = j.value("previous", "");
    outRenderpass->nextPass = j.value("next", "");
    bool doDepth = j["depth"].is_boolean() ? j["depth"].get<bool>():false;

    // Subpass
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS;

    std::vector<VkAttachmentDescription> attachmentDesc;

    // Colour attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format        = state.swapchain.format.format;
    attachmentDescription.samples       = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.initialLayout = outRenderpass->previousPass.empty() ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDescription.finalLayout   = outRenderpass->nextPass.empty() ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    attachmentDescription.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp= VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference attachmentRef = {};
    attachmentRef.attachment    = 0;
    attachmentRef.layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpassDescription.colorAttachmentCount     = 1;
    subpassDescription.pColorAttachments        = &attachmentRef;

    attachmentDesc.push_back(attachmentDescription);

    if(doDepth)
    {
        VkAttachmentDescription depthAttachmentDescription{};
        depthAttachmentDescription.format           = state.swapchain.depthFormat;
        depthAttachmentDescription.samples          = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDescription.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentDescription.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentDescription.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        VkAttachmentReference depthRef{};
        depthRef.attachment = 1;
        depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpassDescription.pDepthStencilAttachment  = &depthRef;

        attachmentDesc.push_back(depthAttachmentDescription);
    }

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

    VkSubpassDependency subpassDependencies[2] = {dependency, depthDependency};

    VkRenderPassCreateInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount    = attachmentDesc.size();
    info.pAttachments       = attachmentDesc.data();
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

bool vulkanCreateRenderPipeline(
    VulkanRenderPipeline* pipeline,  
    const json& j)
{

    // Should we only get the configuration and then create them on the initialization pass
    // TODO Not necessary at the moment, it will be made modular in the future, probably in a separate function.
/*     std::vector<VkShaderStageFlags> shaderStages;
    json jstages = j["stages"];
    for(const json& jstage : jstages.items())
    {
        std::string stageStr = jstage.get<std::string>();
        if(stageStr == "vs")
            shaderStages.push_back(VK_SHADER_STAGE_VERTEX_BIT);
        else if(stageStr == "ps")
            shaderStages.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);
        else
            shaderStages.push_back(VK_SHADER_STAGE_ALL_GRAPHICS);
    }
 */
    std::string renderpassName = j.value("renderpass", "");
    pipeline->renderpass = state.renderpasses[renderpassName + ".renderpass"];
    PASSERT(pipeline->renderpass);

    // Compile shaders --- One for each stage. At the moment only Vertex and Fragment shader.
    const std::string vsFilename = j["stages"].value("vs", "");
    const std::string psFilename = j["stages"].value("ps", "");
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

    // TODO Create shader stages, should be done in separated functions. 
    std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo(2);

    VkPipelineShaderStageCreateInfo vertexStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertexStageInfo.stage   = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module  = pipeline->shaderStages[0].shaderModule;
    vertexStageInfo.pName   = "main";
    shaderStagesInfo.at(0) = (vertexStageInfo);

    VkPipelineShaderStageCreateInfo fragmentStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragmentStageInfo.stage     = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module    = pipeline->shaderStages[1].shaderModule;
    fragmentStageInfo.pName     = "main";
    shaderStagesInfo.at(1) = (fragmentStageInfo);

    // Set the samplers index
    pipeline->samplerUses[0] = TEXTURE_USE_DIFFUSE;
    pipeline->samplerUses[1] = TEXTURE_USE_NORMAL;
    pipeline->samplerUses[2] = TEXTURE_USE_METALLIC_ROUGHNESS;

    // HARDCODED
    pipeline->pipelineConfig.poolSizes[0] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024};
    pipeline->pipelineConfig.poolSizes[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096};

    // Create descriptor pool
    VkDescriptorPoolCreateInfo descriptorPoolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolInfo.poolSizeCount    = 2;
    descriptorPoolInfo.pPoolSizes       = pipeline->pipelineConfig.poolSizes;
    descriptorPoolInfo.maxSets          = 1024; //TODO make this configurable.
    descriptorPoolInfo.flags            = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(vkCreateDescriptorPool(state.device.handle, &descriptorPoolInfo, nullptr, &pipeline->descriptorPool));

    // Global descriptor set config.
    VkDescriptorSetLayoutBinding globalDescriptorSetLayoutBinding{};
    globalDescriptorSetLayoutBinding.binding            = 0;  // TODO Make configurable, even thoug global will always be bound to 0.
    globalDescriptorSetLayoutBinding.descriptorCount    = 1;
    globalDescriptorSetLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalDescriptorSetLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // ! No global samplers at the moment.
    //pipeline->descriptorSets[0].bindings->binding = pipeline->pipelineConfig.vDescriptorSetConfigs.size();  // TODO Make configurable
    //pipeline->descriptorSets[0].bindings->descriptorCount = 1; // TODO Make configurable
    //pipeline->descriptorSets[0].bindings->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //pipeline->descriptorSets[0].bindings->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 1; // TODO At the moment there is only one global binding. It may grow if we have global samples.
    layoutInfo.pBindings    = &globalDescriptorSetLayoutBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(state.device.handle, &layoutInfo, nullptr, &pipeline->descriptorSetLayout[0]));

    u8 localsCount = j.count("local");
    if(j.count("local") > 0)
    {
        for(auto it : j["local"].items())
        {
            const json& jdef = it.value();
            pipeline->pushConstantSize = jdef.value("range", "") == "ModelUniformSize" ? sizeof(glm::mat4) : 0;
            pipeline->pushConstantStride = sizeof(glm::mat4); // TODO Make if from json file ...
        }
    }

    u32 bindingCount = 0;
    //std::vector<VkDescriptorSetLayoutBinding> vLayouts;
    VkDescriptorSetLayoutBinding layouts[2] = {};
    u8 instanceCount = j.count("instance");
    if(instanceCount > 0)
    {
        for(auto instance : j["instance"].items())
        {
            const json& jdef = instance.value();
            //VkDescriptorSetLayoutBinding layout;
            layouts[bindingCount].binding          = bindingCount;
            layouts[bindingCount].descriptorCount  = 1;
            layouts[bindingCount].descriptorType   = jdef.value("sampler", false) ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layouts[bindingCount].stageFlags       = jdef.value("stage", "all") == "ps" ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT; // TODO make "all" viable
            //vLayouts.push_back(layout);
            bindingCount++;
        }

        // ! No samplers at the moment ...

        VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layoutInfo.bindingCount = bindingCount; //vLayouts.size();
        layoutInfo.pBindings    = layouts; //vLayouts.data();
        //VK_CHECK(vkCreateDescriptorSetLayout(state.device.handle, &layoutInfo, nullptr, &pipeline->descriptorSetLayout[1]));
        VkResult r = vkCreateDescriptorSetLayout(state.device.handle, &layoutInfo, nullptr, &pipeline->descriptorSetLayout[1]);
    }

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

    // Get the vertex declaration from the pipeline config.
    const VertexDeclaration* vtx;
    if(j.count("vert_decl") > 0.0f)
    {
        json jattr = j.value("vert_decl", "");
        PASSERT(jattr.is_string())
        vtx = getVertexDeclarationByName(jattr.get<std::string>());
    }
    else
    {
        PFATAL("No vertex declaration in pipeline configuration. Aborting ...");
        return false;
    };

    // Get the push constant ranges from the pipeline config.
    std::vector<VkPushConstantRange> pushConstantRanges;
    if(j.count("local") > 0)
    {
        u32 offset = 0.0f;
        json jlocals = j["local"];
        for(auto& jitems : jlocals)
        {
            json jstage = jitems.value("stage", "");
            std::string stageName = jstage.get<std::string>();
            json jrange = jitems.value("range", "");
            std::string rangeName = jrange.get<std::string>();
            u32 sizeRange = rangeName == "ModelUniformSize" ? sizeof(glm::mat4) : 0;
            VkPushConstantRange p;
            p.size = sizeRange;
            p.offset = offset;
/*             if(stageName == ("vs"))
                p.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            else if(stageName == "ps")
                p.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            else
                p.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS; */
            p.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
            pushConstantRanges.push_back(p);

            offset += sizeRange;
        }
    }

    vulkanCreateGraphicsPipeline(
        state.device,
        pipeline->renderpass,
        vtx->size,
        vtx->layout,
        shaderStagesInfo.size(),
        shaderStagesInfo.data(),
        2,
        pipeline->descriptorSetLayout,
        pushConstantRanges,
        1,
        &colorBlendAttachment,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        viewport,
        scissors,
        false,
        true,
        pipeline
    );

    // Set all materialInstances ids to 0.
    for(u32 i = 0; i < VULKAN_MAX_MATERIAL_COUNT; ++i)
    {
        pipeline->materialInstances[i].id = INVALID_ID;
    }

    // Uniform buffer // TODO Make dynamic.
    pipeline->uboAlignment      = state.device.properties.limits.minUniformBufferOffsetAlignment;
    pipeline->globalUboStride   = getAligned(sizeof(GlobalUniformBufferData), pipeline->uboAlignment);
    pipeline->instanceUboStride = getAligned(sizeof(VulkanMaterialInstance), pipeline->uboAlignment);

    u64 totalSize = pipeline->globalUboStride + (pipeline->instanceUboStride * VULKAN_MAX_MATERIAL_COUNT);
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
        vulkanRenderTargetCreate(state.device, attachments, pipeline->renderpass, state.swapchain.extent.width, state.swapchain.extent.height, renderTarget);
        pipeline->renderpass->renderTargets.emplace_back(*renderTarget);
    }
    
    VK_CHECK(vkAllocateDescriptorSets(state.device.handle, &descriptorSetAllocInfo, pipeline->globalDescriptorSet));
    return true;
}

void vulkanDestroyRenderPipeline(VulkanRenderPipeline* pipeline)
{
    if(!pipeline){
        PERROR("vulkanPipelineDestroy - No pipeline to destroy.");
        return;
    }

    for(u8 i = 0; i < 2/* pipeline->descriptorSetCount */; ++i){
        vkDestroyDescriptorSetLayout(state.device.handle, pipeline->descriptorSetLayout[i], nullptr);
    }

    if(pipeline->descriptorPool)
        vkDestroyDescriptorPool(state.device.handle, pipeline->descriptorPool, nullptr);

    vulkanBufferDestroy(state.device, pipeline->uniformBuffer);

    vulkanRenderPassDestroy(pipeline->renderpass);

    if(pipeline->pipeline.handle)
    {
        vkDestroyPipeline(state.device.handle, pipeline->pipeline.handle, nullptr);
        pipeline->pipeline.handle = nullptr;
    }
    if(pipeline->pipeline.layout)
    {
        vkDestroyPipelineLayout(state.device.handle, pipeline->pipeline.layout, nullptr);
        pipeline->pipeline.layout = nullptr;
    }

    for(auto stage : pipeline->shaderStages)
        vkDestroyShaderModule(state.device.handle, stage.shaderModule, nullptr);

}

void vulkanUsePipeline(const std::string& name)
{
    vulkanBindPipeline(&state.commandBuffers[state.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipelines[name]);
}

void vulkanBindPipeline(CommandBuffer* cmd, VkPipelineBindPoint bindPoint, VulkanRenderPipeline* pipeline)
{
    vkCmdBindPipeline(cmd->handle, bindPoint, pipeline->pipeline.handle);
    state.boundPipeline = pipeline;
}

bool vulkanActivateGlobals()
{
    if(vulkanBindGlobals())
        return vulkanApplyGlobals();
    return false;
}

bool vulkanActivateConstants(const RenderMeshData* mesh)
{
    vkCmdPushConstants(
        state.commandBuffers[state.imageIndex].handle, 
        state.boundPipeline->pipeline.layout, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, state.boundPipeline->pushConstantSize, 
        &mesh->model);
    return true;
}

bool vulkanActivateInstance(const RenderMeshData* mesh)
{
    u32 idx = state.imageIndex;
    u32 descriptorCount = 0;
    u32 descriptorIdx = 0;

    u32 range = state.boundPipeline->instanceUboStride;
    u32 offset = state.boundPipeline->globalUboStride + range * mesh->material->rendererId;

    VkWriteDescriptorSet writes[4];

    VulkanMaterialShaderUBO data{};
    data.diffuseColor = mesh->material->diffuseColor;
    vulkanBufferLoadData(state.device, state.boundPipeline->uniformBuffer, offset, range, 0, &data);

    VulkanMaterialInstance* descriptor = &state.boundPipeline->materialInstances[mesh->material->rendererId];

    if(descriptor->descriptorState->generations[idx] == INVALID_ID ||
        descriptor->descriptorState->generations[idx] != mesh->material->generation)
    {
        VkDescriptorBufferInfo info{};
        info.buffer = state.boundPipeline->uniformBuffer.handle;
        info.offset = offset;
        info.range  = range;

        VkWriteDescriptorSet instanceWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        instanceWrite.pBufferInfo       = &info;
        instanceWrite.descriptorCount   = 1;
        instanceWrite.descriptorType    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        instanceWrite.dstArrayElement   = 0;
        instanceWrite.dstBinding        = 0;
        instanceWrite.dstSet            = descriptor->descriptorSets[idx];

        writes[descriptorCount] = instanceWrite;
        descriptorCount++;
        descriptor->descriptorState[0].generations[idx] = mesh->material->generation;
    }
    descriptorIdx++;

    // ! Make sampler bindings ...
    Material* m = mesh->material;
    const u32 samplerCount = 1;
    for(u32 samplerIdx = 0; samplerIdx < samplerCount; ++samplerIdx)
    {
        TextureUse use = (TextureUse)state.boundPipeline->samplerUses[samplerIdx];
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

        VkDescriptorImageInfo imageInfo[3];
        u32* descriptorGeneration = &descriptor->descriptorState[1].generations[idx];
        u32* descriptorId = &descriptor->descriptorState[1].ids[idx];

        if(t && (*descriptorId != t->id || *descriptorGeneration == INVALID_ID || t->generation != *descriptorGeneration))
        {
            VulkanTexture* vulkanTexture = (VulkanTexture*)t->data;

            imageInfo[samplerIdx].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[samplerIdx].imageView     = vulkanTexture->image.view;
            imageInfo[samplerIdx].sampler       = vulkanTexture->sampler;

            VkWriteDescriptorSet textWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            textWrite.descriptorCount   = 1;
            textWrite.pImageInfo        = &imageInfo[samplerIdx];
            textWrite.descriptorType    = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textWrite.dstArrayElement   = 0;
            textWrite.dstBinding        = descriptorIdx;
            textWrite.dstSet            = descriptor->descriptorSets[idx];

            writes[descriptorCount] = textWrite;
            descriptorCount++;

            if(t->generation != INVALID_ID) {
                *descriptorGeneration = t->generation;
                *descriptorId = t->id;
            }
        }
        descriptorIdx++;
    }

    if(descriptorCount > 0)
        vkUpdateDescriptorSets(state.device.handle, descriptorCount, writes, 0, nullptr);

    vkCmdBindDescriptorSets(
        state.commandBuffers[idx].handle, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        state.boundPipeline->pipeline.layout,
        1, 1, &descriptor->descriptorSets[idx], 
        0, nullptr);
    return true;
}

bool vulkanBindGlobals()
{
    if(state.boundPipeline == nullptr)
    {
        PFATAL("There is no pipeline bound.");
        return false;
    }
    return true;
}

bool vulkanApplyGlobals()
{

    // TODO This should be in another function call.
    CEntity* hcamera = getEntityByName("camera");
    TCompCamera* cCamera = hcamera->get<TCompCamera>();

    GlobalUniformBufferData data;
    data.view        = cCamera->getView();
    data.projection  = cCamera->getProjection();
    data.position    = cCamera->getEye();

    VulkanRenderPipeline* pipeline = state.boundPipeline;

    vulkanBufferLoadData(state.device, state.boundPipeline->uniformBuffer, 0, pipeline->globalUboStride, 0, &data);

    // ------------------------------------------

    u32 imageIdx = state.imageIndex;
    VkCommandBuffer cmdBuffer = state.commandBuffers[imageIdx].handle;
    VkDescriptorSet descriptor = pipeline->globalDescriptorSet[imageIdx];

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer   = pipeline->uniformBuffer.handle;
    bufferInfo.offset   = 0; // This is hardcoded since globals will always be stored at the beginning of the buffer.
    bufferInfo.range    = pipeline->globalUboStride;

    VkWriteDescriptorSet uboWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    uboWrite.dstSet             = pipeline->globalDescriptorSet[imageIdx];
    uboWrite.dstBinding         = 0;
    uboWrite.dstArrayElement    = 0;
    uboWrite.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount    = 1;
    uboWrite.pBufferInfo        = &bufferInfo;

    VkWriteDescriptorSet descriptorWrites[2];
    descriptorWrites[0] = uboWrite;

    vkUpdateDescriptorSets(state.device.handle, 1, descriptorWrites, 0, 0);

    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline.layout, 0, 1, &descriptor, 0, 0);
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