#include "vulkanSwapchain.h"

static bool create(VulkanState* pState, u32 width, u32 height);
static void destroy(VulkanState* pState);

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

bool vulkanSwapchainCreate(VulkanState* pState)
{
    VkExtent2D swapchainExtent = getSwapchainExtent(pState);
    return create(pState, swapchainExtent.width, swapchainExtent.height);
}

void vulkanSwapchainDestroy(VulkanState* pState)
{
    destroy(pState);
}

bool vulkanSwapchainRecreate(
    VulkanState* pState, 
    u32 width, 
    u32 height)
{
    destroy(pState);
    return create(pState, width, height);
}

static bool create(
    VulkanState* pState, 
    u32 width, 
    u32 height)
{
    pState->swapchain.extent = { width, height };

    // Choose the format for the swapchain.
    pState->swapchain.format = pState->swapchainSupport.formats[0];
    for(u32 i = 0; i < pState->swapchainSupport.formatCount; ++i)
    {
        if(pState->swapchainSupport.formats[i].format == VK_FORMAT_B8G8R8_SRGB &&
            pState->swapchainSupport.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
                pState->swapchain.format = pState->swapchainSupport.formats[i];
                break;
            }
    }

    // Choose present mode.
    pState->swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(u32 i = 0; i < pState->swapchainSupport.presentModeCount; ++i)
    {
        if(pState->swapchainSupport.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
            pState->swapchain.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    pState->swapchain.imageCount        = pState->swapchainSupport.capabilities.minImageCount + 1;
    pState->swapchain.maxImageInFlight  = pState->swapchain.imageCount - 1;
    pState->swapchain.framebuffers.resize(pState->swapchain.imageCount);

    VkSwapchainCreateInfoKHR swapchainInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainInfo.surface           = pState->surface;
    swapchainInfo.minImageCount     = pState->swapchain.imageCount;
    swapchainInfo.imageFormat       = pState->swapchain.format.format;
    swapchainInfo.imageColorSpace   = pState->swapchain.format.colorSpace;
    swapchainInfo.imageExtent       = pState->swapchain.extent;
    swapchainInfo.imageArrayLayers  = 1;
    swapchainInfo.imageUsage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.presentMode       = pState->swapchain.presentMode;

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

    VkResult result = vkCreateSwapchainKHR(pState->device.handle, &swapchainInfo, nullptr, &pState->swapchain.handle);
    if(result != VK_SUCCESS)
    {
        PFATAL("Failed to create swapchain! Shutting down.");
        return false;
    }
    
    pState->currentFrame = 0;

    VK_CHECK(vkGetSwapchainImagesKHR(pState->device.handle, pState->swapchain.handle, &pState->swapchain.imageCount, nullptr));
    pState->swapchain.images.resize(pState->swapchain.imageCount);
    pState->swapchain.imageViews.resize(pState->swapchain.imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(pState->device.handle, pState->swapchain.handle, &pState->swapchain.imageCount, pState->swapchain.images.data()));

    for(u32 i = 0; i < pState->swapchain.imageCount; ++i)
    {
        VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        info.image                              = pState->swapchain.images.at(i);
        info.viewType                           = VK_IMAGE_VIEW_TYPE_2D;
        info.format                             = pState->swapchain.format.format;
        info.subresourceRange.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel      = 0;
        info.subresourceRange.levelCount        = 1;
        info.subresourceRange.baseArrayLayer    = 0;
        info.subresourceRange.layerCount        = 1;

        VK_CHECK(vkCreateImageView(pState->device.handle, &info, nullptr, &pState->swapchain.imageViews.at(i)));
    }

    PINFO("Swapchain created successfully.");
    return true;
};

/**
 * @brief Waits for all device queues to finish their process
 * and proceeds to destroy all swapchain imageViews and the swapchain
 * itself.
 */ 
static void destroy(VulkanState *pState)
{
    vkDeviceWaitIdle(pState->device.handle);

    for(const auto& image : pState->swapchain.imageViews){
        vkDestroyImageView(pState->device.handle, image, nullptr);
    }

    vkDestroySwapchainKHR(pState->device.handle, pState->swapchain.handle, nullptr);
};