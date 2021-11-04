#include "vulkanRenderpass.h"

/**
 * @brief Right now creates the main hardcoded renderpass.
 * @param VulkanState* pState
 * @return bool if succeded.
 */
bool vulkanRenderPassCreate(VulkanState* pState)
{
    // Colour attachment
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format        = pState->swapchain.format.format;
    attachmentDescription.samples       = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachmentDescription.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp= VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference attachmentRef = {};
    attachmentRef.attachment    = 0;
    attachmentRef.layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments    = &attachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass       = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass       = 0;
    dependency.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask    = 0;
    dependency.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount    = 1;
    info.pAttachments       = &attachmentDescription;
    info.subpassCount       = 1;
    info.pSubpasses         = &subpassDescription;
    info.dependencyCount    = 1;
    info.pDependencies      = &dependency;

    if(vkCreateRenderPass(pState->device.handle, &info, nullptr, &pState->renderpass.handle) != VK_SUCCESS)
    {
        PFATAL("Render pass could not be created!");
        return false;
    }

    PINFO("Main renderpass created!");
    return true;
}

/**
 * @brief Destroys the main render pass.
 * @param VulkanState* pState
 * @return void
 */
void vulkanRenderPassDestroy(VulkanState* pState)
{
    vkDestroyRenderPass(pState->device.handle, pState->renderpass.handle, nullptr);
}