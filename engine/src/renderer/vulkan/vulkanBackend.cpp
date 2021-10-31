#include "vulkanBackend.h"
#include "vulkanTypes.h"
#include "vulkanDevice.h"

#include "core\application.h"

#include "platform\platform.h"
#include <vector>
#include <string>

static VulkanState state;

bool vulkanCreateSwapchain(VulkanState* pState);
bool vulkanRecreateSwapchain(VulkanState* pState);

VkResult vulkanCreateDebugMessenger(VulkanState* pState);

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
    if(!vulkanCreateSwapchain(&state)){
        return false;
    }

    // Create Semaphores

    // Create render passes

    // Create Graphics pipeline


    return true;
}

void vulkanBackendOnResize()
{
    vulkanRecreateSwapchain(&state);
}

void vulkanBackendShutdown()
{
    vkDestroyInstance(state.instance, nullptr);
}

bool vulkanBeginFrame()
{
    return true;
}

bool vulkanDraw()
{
    return true;
}

void vulkanEndFrame()
{

}

VkExtent2D getSwapchainExtent(VulkanState* pState)
{
    if(pState->swapchainSupport.capabilities.currentExtent.width != UINT32_MAX){
        return pState->swapchainSupport.capabilities.currentExtent;
    } else {
        u32 width = 0, height = 0;
        VkExtent2D extent = {width, height};
        extent.width = PCLAMP(width, pState->swapchainSupport.capabilities.minImageExtent.width, 
            pState->swapchainSupport.capabilities.maxImageExtent.width);
        extent.height = PCLAMP(height, pState->swapchainSupport.capabilities.minImageExtent.height,
            pState->swapchainSupport.capabilities.maxImageExtent.height);
        return extent;
    }
}

bool vulkanCreateSwapchain(VulkanState* pState)
{
    VkExtent2D extent = getSwapchainExtent(pState);

    // Choose the format for the swapchain.
    u32 formatIndex = 0;
    for(u32 i = 0; i < pState->swapchainSupport.formatCount; ++i)
    {
        if(pState->swapchainSupport.formats[i].format == VK_FORMAT_B8G8R8_SRGB &&
            pState->swapchainSupport.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
                formatIndex = i;
                break;
            }
    }

    // Choose present mode.
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(u32 i = 0; i < pState->swapchainSupport.presentModeCount; ++i)
    {
        if(pState->swapchainSupport.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainInfo.surface           = pState->surface;
    swapchainInfo.minImageCount     = pState->swapchainSupport.capabilities.minImageCount + 1;
    swapchainInfo.imageFormat       = pState->swapchainSupport.formats[formatIndex].format;
    swapchainInfo.imageColorSpace   = pState->swapchainSupport.formats[formatIndex].colorSpace;
    swapchainInfo.imageExtent       = extent;
    swapchainInfo.imageArrayLayers  = 1;
    swapchainInfo.imageUsage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.presentMode       = presentMode;

    if(pState->device.graphicsQueueIndex == pState->device.presentQueueIndex) {
        swapchainInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices   = nullptr;
    } else {
        std::vector<u32> familyIndices = {pState->device.graphicsQueueIndex, pState->device.presentQueueIndex};
        swapchainInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices   = familyIndices.data();
    }

    swapchainInfo.preTransform      = pState->swapchainSupport.capabilities.currentTransform;
    swapchainInfo.compositeAlpha    = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.clipped           = VK_TRUE;
    swapchainInfo.oldSwapchain      = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(pState->device.device, &swapchainInfo, nullptr, &pState->swapchain);
    if(result != VK_SUCCESS)
    {
        PFATAL("Failed to create swapchain! Shutting down.");
        return false;
    }
    PINFO("Swapchain created.");
    return true;

}

void vulkanDestroySwapchain(VulkanState* pState)
{
    vkDestroySwapchainKHR(pState->device.device, pState->swapchain, nullptr);
}

bool vulkanRecreateSwapchain(VulkanState* pState)
{
    vulkanDestroySwapchain(pState);
    return vulkanCreateSwapchain(pState);
}

VkResult vulkanCreateDebugMessenger(VulkanState* pState)
{
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
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