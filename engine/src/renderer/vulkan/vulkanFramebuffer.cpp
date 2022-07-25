#include "vulkanFramebuffer.h"

bool vulkanRenderTargetCreate(
    const VulkanDevice& device,
    std::vector<VkImageView>& attachments,
    VulkanRenderpass* renderpass,
    u32 width, u32 height,
    VulkanRenderTarget* renderTarget)
{
    for(auto att : attachments)
        renderTarget->attachments.push_back(att);
    
    VkFramebufferCreateInfo info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    info.attachmentCount    = attachments.size();
    info.pAttachments       = attachments.data();
    info.renderPass         = renderpass->handle;
    info.width              = width;
    info.height             = height;
    info.layers             = 1;
    
    VK_CHECK(vkCreateFramebuffer(device.handle, &info, nullptr, &renderTarget->handle));
    return true;
}

void vulkanRenderTargetDestroy(
    const VulkanDevice& device,
    VulkanRenderTarget* target)
{
    if(target )
    {
        vkDestroyFramebuffer(device.handle, target->handle, nullptr);
        if(target->attachments.empty() == false)
            target->attachments.clear();
    }
}