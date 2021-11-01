#include "vulkanFramebuffer.h"

bool vulkanFramebufferCreate(
    VulkanState* pState, 
    VulkanRenderpass* renderpass, 
    u32 width, u32 height, 
    u32 attachmentCount,
    std::vector<VkImageView>& attachments,
    Framebuffer* outFramebuffer)
{
    for(u32 i = 0; i < attachmentCount; ++i){
        outFramebuffer->attachments.push_back(attachments.at(i));
    }
    outFramebuffer->renderpass = renderpass;

    VkFramebufferCreateInfo info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    info.attachmentCount    = attachmentCount;
    info.pAttachments       = attachments.data();
    info.renderPass         = renderpass->handle;
    info.width              = width;
    info.height             = height;
    info.layers             = 1;

    VK_CHECK(vkCreateFramebuffer(pState->device.handle, &info, nullptr, &outFramebuffer->handle));
    return true;
}

void vulkanFramebufferDestroy(VulkanState* pState, Framebuffer* framebuffer)
{
    vkDestroyFramebuffer(pState->device.handle, framebuffer->handle, nullptr);
    if(!framebuffer->attachments.empty())
    {
        framebuffer->attachments.clear();
    }
    framebuffer->handle = nullptr;
    framebuffer->renderpass = nullptr;
}