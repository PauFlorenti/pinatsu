#pragma once

#include "vulkan\vulkan.h"
#include "core\assert.h"
#include <vector>
#include "core\logger.h"

#define VK_CHECK(x) { PASSERT(x == VK_SUCCESS); }

typedef struct VulkanDevice
{ 
    VkPhysicalDevice    physicalDevice;
    VkDevice            device;

    u32 graphicsQueueIndex;
    u32 presentQueueIndex;
    u32 transferQueueIndex;
    u32 computeQueueIndex;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkCommandPool commandPool;

} VulkanDevice;

typedef struct VulkanSwapchainSupport
{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
} VulkanSwapchainSupport;

typedef struct VulkanState
{
    VkInstance      instance;
    VulkanDevice    device;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkSurfaceKHR    surface;

    // TODO implement own allocator for vulkan
    // VkAllocationCallbacks* allocator;

    u32 clientWidth;
    u32 clientHeight;

    VulkanSwapchainSupport swapchainSupport{};
    VkSwapchainKHR swapchain;
} VulkanState;