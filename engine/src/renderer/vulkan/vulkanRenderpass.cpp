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

    VkAttachmentDescription depthAttachmentDescription{};
    depthAttachmentDescription.format           = pState->swapchain.depthFormat;
    depthAttachmentDescription.samples          = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDescription.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDescription.finalLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentDescription.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDescription.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference attachmentRef = {};
    attachmentRef.attachment    = 0;
    attachmentRef.layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount     = 1;
    subpassDescription.pColorAttachments        = &attachmentRef;
    subpassDescription.pDepthStencilAttachment  = &depthRef;

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

    VkAttachmentDescription attachmentDesc[2] = {attachmentDescription, depthAttachmentDescription};
    VkSubpassDependency subpassDependencies[2] = {dependency, depthDependency};

    VkRenderPassCreateInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount    = 2;
    info.pAttachments       = attachmentDesc;
    info.subpassCount       = 1;
    info.pSubpasses         = &subpassDescription;
    info.dependencyCount    = 2;
    info.pDependencies      = subpassDependencies;

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