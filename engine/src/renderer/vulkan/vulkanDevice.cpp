#include "vulkanDevice.h"
#include "memory\pmemory.h"

typedef struct PhysicalDeviceRequirements
{
    bool graphics;
    bool present;
    bool transfer;
    bool compute;
    std::vector<const char*> extensionNames;
    bool discrete;
    bool samplerAnisotropy;
} PhysicalDeviceRequirements;

typedef struct QueueFamilyIndexInfo
{
    u32 GraphicsQueueFamilyIndex;
    u32 PresentQueueFamilyIndex;
    u32 TransferQueueFamilyIndex;
    u32 ComputeQueueFamilyIndex;
} QueueFamilyIndexInfo;

void querySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupport* outSwapchainSupport)
{
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &outSwapchainSupport->capabilities
    ));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &outSwapchainSupport->formatCount,
        0
    ));

    if(outSwapchainSupport->formatCount != 0)
    {
        if(!outSwapchainSupport->formats)
        {
            outSwapchainSupport->formats = (VkSurfaceFormatKHR*)memAllocate(
                    sizeof(VkSurfaceFormatKHR) * outSwapchainSupport->formatCount,
                    MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice, surface,
            &outSwapchainSupport->formatCount,
            outSwapchainSupport->formats
        ));
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, surface,
        &outSwapchainSupport->presentModeCount, nullptr
    ));

    if(outSwapchainSupport->presentModeCount != 0)
    {
        if(!outSwapchainSupport->presentModes)
        {
            outSwapchainSupport->presentModes = (VkPresentModeKHR*)memAllocate(
                sizeof(VkPresentModeKHR) * outSwapchainSupport->presentModeCount,
                MEMORY_TAG_RENDERER
            );
        }

        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice, surface,
            &outSwapchainSupport->presentModeCount,
            outSwapchainSupport->presentModes
        ));
    }
}

bool physicalDeviceMeetsRequirements(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* deviceProperties,
    const VkPhysicalDeviceFeatures* deviceFeatures,
    const PhysicalDeviceRequirements* requirements,
    QueueFamilyIndexInfo* outFamilyIndexInfo,
    VulkanSwapchainSupport* outSwapchainSupport
)
{
    outFamilyIndexInfo->ComputeQueueFamilyIndex     = -1;
    outFamilyIndexInfo->GraphicsQueueFamilyIndex    = -1;
    outFamilyIndexInfo->PresentQueueFamilyIndex     = -1;
    outFamilyIndexInfo->TransferQueueFamilyIndex    = -1;

    if(requirements->discrete && 
        deviceProperties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        PINFO("Device is not a discrete GPU. One is required. Skipping");
        return false;   
    }

    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    u32 queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
    queueFamilyProperties.resize(queueFamilyPropertiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, 
                                             queueFamilyProperties.data());

    u8 minTransferScore = 255;
    PINFO("Graphics | Present | Transfer | Compute | Name ");
    for(u32 i = 0; i < queueFamilyPropertiesCount; ++i)
    {
        u8 currentTransferScore = 0;
        // Graphics ??
        if(queueFamilyProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            outFamilyIndexInfo->GraphicsQueueFamilyIndex = i;
            currentTransferScore++;

            // If it also is a present queue, we prioritize the grouping of the 2 queues.
            // Present ??
            VkBool32 surfaceSupported = VK_FALSE;
            VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &surfaceSupported));
            if(surfaceSupported){
                outFamilyIndexInfo->PresentQueueFamilyIndex = i;
                currentTransferScore++;
            }
        }

        // Compute ??
        if(queueFamilyProperties.at(i).queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            outFamilyIndexInfo->ComputeQueueFamilyIndex = i;
            currentTransferScore++;
        }

        // Transfer ??
        if(queueFamilyProperties.at(i).queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            if(currentTransferScore <= minTransferScore) {
                minTransferScore = currentTransferScore;
                outFamilyIndexInfo->TransferQueueFamilyIndex = i;
            }
        }

        // If a present queue hasn't been found, iterate again and take the first one.
        // This should only happen if and only if there is a queue that supports graphics but 
        // NOT present. 
        if(outFamilyIndexInfo->PresentQueueFamilyIndex == -1){
            for(u32 i = 0; i < queueFamilyPropertiesCount; ++i){
                VkBool32 supportsPresent = VK_FALSE;
                VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent));
                if(supportsPresent){
                    outFamilyIndexInfo->PresentQueueFamilyIndex = i;
                    
                    // Warn about graphics and present queue being different.
                    if(outFamilyIndexInfo->PresentQueueFamilyIndex != outFamilyIndexInfo->GraphicsQueueFamilyIndex){
                        PWARN("Warning! Different queue index used for present and graphics: %u\n", i);
                    }
                    break;
                }
            }
        }
    }

    PINFO("\t%d| \t%d | \t%d | \t%d | %s",
        outFamilyIndexInfo->GraphicsQueueFamilyIndex,
        outFamilyIndexInfo->PresentQueueFamilyIndex,
        outFamilyIndexInfo->TransferQueueFamilyIndex,
        outFamilyIndexInfo->ComputeQueueFamilyIndex,
        deviceProperties->deviceName
    );

    if(
        (!requirements->graphics || (requirements->graphics && outFamilyIndexInfo->GraphicsQueueFamilyIndex != -1)) &&
        (!requirements->present || (requirements->present && outFamilyIndexInfo->PresentQueueFamilyIndex != -1)) &&
        (!requirements->transfer || (requirements->transfer && outFamilyIndexInfo->TransferQueueFamilyIndex != -1)) &&
        (!requirements->compute || (requirements->compute && outFamilyIndexInfo->ComputeQueueFamilyIndex != -1)))
    {   
        PINFO("Device meets requirements!");
        PINFO("Graphics Family Index: %d", outFamilyIndexInfo->GraphicsQueueFamilyIndex);
        PINFO("Present Family Index: %d", outFamilyIndexInfo->PresentQueueFamilyIndex);
        PINFO("Transfer Family Index: %d", outFamilyIndexInfo->TransferQueueFamilyIndex);
        PINFO("Compute Family Index: %d", outFamilyIndexInfo->ComputeQueueFamilyIndex);
        // Swapchain support?
        querySwapchainSupport(physicalDevice, surface, outSwapchainSupport);

        if(outSwapchainSupport->formatCount < 1 || outSwapchainSupport->presentModeCount < 1){
            if(outSwapchainSupport->formats){
                memFree(outSwapchainSupport->formats, 
                    sizeof(VkSurfaceFormatKHR) * outSwapchainSupport->formatCount, 
                    MEMORY_TAG_RENDERER);
            }
            if(outSwapchainSupport->presentModes)
            {
                memFree(outSwapchainSupport->presentModes,
                    sizeof(VkPresentModeKHR) * outSwapchainSupport->presentModeCount,
                    MEMORY_TAG_RENDERER);
            }
            PINFO("Required swapchain support not present, skipping device.");
            return false;
        }

        // Device extensions
        if(!requirements->extensionNames.empty())
        {
            u32 availableExtensionCount = 0;
            std::vector<VkExtensionProperties> availableExtensions;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                physicalDevice,
                nullptr, 
                &availableExtensionCount, 
                nullptr)
            );
            if(availableExtensionCount != 0)
            {
                availableExtensions.resize(availableExtensionCount);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    physicalDevice, 
                    nullptr,
                    &availableExtensionCount,
                    availableExtensions.data()
                ));

                for(const char* required : requirements->extensionNames)
                {
                    bool found = false;
                    for(const VkExtensionProperties available : availableExtensions)
                    {
                        if(strcmp(required, available.extensionName) == 0)
                        {
                            found = true;
                            break;
                        }
                    }

                    if(!found)
                    {
                        PINFO("Required extension not found: '%s', skipping device.", required);
                        return false;
                    }
                }
            }
        }

        // Sampler anisotropy
        if(requirements->samplerAnisotropy && !deviceFeatures->samplerAnisotropy){
            PINFO("Device does not support samplerAnisotropy, skipping device.");
            return false;
        }
        return true;
    }
    return false;
}

bool pickPhysicalDevice(VulkanState* state)
{
    // Enumerate all available physical devices in the system.
    u32 physicalDeviceCount = 0;
    std::vector<VkPhysicalDevice> physicalDevices;

    VK_CHECK(vkEnumeratePhysicalDevices(state->instance, &physicalDeviceCount, nullptr));
    if(physicalDeviceCount > 0)
    {
        physicalDevices.resize(physicalDeviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(state->instance, &physicalDeviceCount, 
                                            physicalDevices.data()));
    }
    else{
        PFATAL("No available physical device in the system.");
        return false;
    }

    PhysicalDeviceRequirements requirements = {};
    requirements.graphics   = true;
    requirements.compute    = false;
    requirements.transfer   = true;
    requirements.present    = true;
    requirements.discrete   = false;
    requirements.samplerAnisotropy = true;
    requirements.extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    //requirements.extensionNames.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);

    QueueFamilyIndexInfo familyIndexInfo;

    for(const VkPhysicalDevice gpu : physicalDevices)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(gpu, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(gpu, &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(gpu, &memory);

        bool result = physicalDeviceMeetsRequirements(
            gpu,
            state->surface,
            &properties,
            &features,
            &requirements,
            &familyIndexInfo,
            &state->swapchainSupport
        );

        if(result)
        {
            // TODO Show picked gpu information.
            PINFO("Selected device: '%s'.", properties.deviceName);

            switch (properties.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                PINFO("GPU type is Unknown.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                PINFO("GPU type is Discrete.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                PINFO("GPU type is Virtual");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                PINFO("GPU type is Integrated.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                PINFO("GPU type is CPU");
                break;
            default:
                break;
            }
            
            // Driver version
            PINFO("GPU Driver Version %d.%d.%d", 
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion)
            );

            // Vulkan API version
            PINFO("Vulkan API Version %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion)
            );

            // Memory info
            for(u32 i = 0; i < memory.memoryHeapCount; ++i)
            {
                f32 memorySizeGB = (((f32)memory.memoryHeaps[i].size) / 1024.f / 1024.f / 1024.f);
                if(memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT){
                    PINFO("Local GPU memory: %.2fGiB", memorySizeGB);
                }
                else {
                    PINFO("Shared System memory: %.2fGiB", memorySizeGB);
                }
            }

            state->device.physicalDevice = gpu;
            state->device.graphicsQueueIndex = familyIndexInfo.GraphicsQueueFamilyIndex;
            state->device.presentQueueIndex = familyIndexInfo.PresentQueueFamilyIndex;
            state->device.transferQueueIndex = familyIndexInfo.TransferQueueFamilyIndex;
            // Set compute if needed

            // Save copy of properties, features and memory for later use.
            state->device.properties = properties;
            state->device.features = features;
            state->device.memory = memory;

            break;
        }
    }

    if(!state->device.physicalDevice){
        PERROR("No physical device was found that met the requirements.");
        return false;
    }

    PINFO("Physical device selected.");
    return true;
};

bool createLogicalDevice(VulkanState* state)
{
    if(!pickPhysicalDevice(state)){
        return false;
    }

    PINFO("Creating logical device ...");

    bool presentShareGraphicsQueue = 
        state->device.presentQueueIndex == state->device.graphicsQueueIndex;
    bool transferShareGraphicsQueue = 
        state->device.transferQueueIndex == state->device.graphicsQueueIndex;
    
    u32 indexCount = 1;
    if(!presentShareGraphicsQueue)
        indexCount++;

    if(!transferShareGraphicsQueue)
        indexCount++;
    
    u32 indices[32];
    u8 index = 0;
    indices[index++] = state->device.graphicsQueueIndex;
    if(!presentShareGraphicsQueue){
        indices[index++] = state->device.presentQueueIndex;
    }
    if(!transferShareGraphicsQueue){
        indices[index++] = state->device.transferQueueIndex;
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
    for(u32 i = 0; i < indexCount; ++i)
    {
        VkDeviceQueueCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex   = indices[i];
        info.queueCount         = 1;
        f32 queuePriority       = 1.0f;
        info.pQueuePriorities   = &queuePriority;

        queueCreateInfo.push_back(info);
    }

    // Request device features.
    VkPhysicalDeviceFeatures deviceFeatures = {};
    // Should be config driven, depending on the requirements.
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    //VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
    //extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    //extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;

    std::vector<const char*> extensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount       = queueCreateInfo.size();
    deviceCreateInfo.pQueueCreateInfos          = queueCreateInfo.data();
    deviceCreateInfo.pEnabledFeatures           = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount      = extensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames    = extensionNames.data();
    //deviceCreateInfo.pNext = &extendedDynamicStateFeatures;

    VK_CHECK(vkCreateDevice(
        state->device.physicalDevice,
        &deviceCreateInfo,
        nullptr,
        &state->device.handle
    ));

    PINFO("Logical Device created.");

    // Get queues
    vkGetDeviceQueue(state->device.handle, state->device.graphicsQueueIndex,
    0 , &state->device.graphicsQueue);
    vkGetDeviceQueue(state->device.handle, state->device.transferQueueIndex,
    0 , &state->device.transferQueue);
    vkGetDeviceQueue(state->device.handle, state->device.presentQueueIndex,
    0 , &state->device.presentQueue);
    //vkGetDeviceQueue(state->device.handle, state->device.graphicsQueueIndex,
    //0 , &state->device.graphicsQueue);

    PINFO("Queues adcquired.");

    // Create command pool for graphics queue.
    VkCommandPoolCreateInfo cmdPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    cmdPoolInfo.queueFamilyIndex = state->device.graphicsQueueIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(state->device.handle, &cmdPoolInfo, 
        nullptr, &state->device.commandPool));
    PINFO("Graphics command pool created.");

    // Create comman pool for transfer queue.
    cmdPoolInfo.queueFamilyIndex = state->device.transferQueueIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    VK_CHECK(vkCreateCommandPool(state->device.handle, &cmdPoolInfo, nullptr, &state->device.transferCmdPool));
    PINFO("Transfer command pool created");

    return true;
}

/**
 * @brief Destroy the logical device, make sure all
 * resources created or allocated from the device have
 * been freed or destroy previous to this.
 * @param VulkanState& pState The Vulkan State
 * @return void
 */
void destroyLogicalDevice(VulkanState& pState)
{
    vkDestroyCommandPool(
        pState.device.handle,
        pState.device.transferCmdPool,
        nullptr);
    
    vkDestroyCommandPool(
        pState.device.handle,
        pState.device.commandPool,
        nullptr);

    vkDeviceWaitIdle(pState.device.handle);
    vkDestroyDevice(pState.device.handle, nullptr);
}

static VkFormat
findSupportedFormat(
    const VkPhysicalDevice& physicalDevice, 
    VkFormat* candidates, 
    VkImageTiling tiling, 
    VkFormatFeatureFlags features)
{
    for(u32 i = 0; i < 3; ++i)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &properties);
        if( tiling == VK_IMAGE_TILING_LINEAR && 
            (properties.linearTilingFeatures & features ) == features)
        {
            return candidates[i];
        }
        else if(tiling == VK_IMAGE_TILING_OPTIMAL && 
                (properties.linearTilingFeatures & features) == features)
        {
            return candidates[i];
        }
    }
    PERROR("Not valid depth format found!");
    return VK_FORMAT_D32_SFLOAT;
}

void
vulkanDeviceGetDepthFormat(VulkanState& state)
{
    VkFormat candidates[3] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    state.swapchain.depthFormat = findSupportedFormat(
            state.device.physicalDevice, 
            candidates, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}