#include "vulkanUtils.h"

i32 
findMemoryIndex(
    VulkanState* pState,
    u32 typeFilter, 
    VkMemoryPropertyFlags memFlags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(pState->device.physicalDevice, &memProperties);
    for(u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if(typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & memFlags) == memFlags){
            return i;
        }
    }
    return -1;
}