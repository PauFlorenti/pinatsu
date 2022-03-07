#pragma once

#include "vulkanTypes.h"
#include "renderer/renderTypes.h"

void
imguiInit(
    VulkanState* state, 
    const VulkanRenderpass* renderpass);

void
imguiRender(
    VkCommandBuffer& cmd, 
    const RenderPacket& packet);

void
imguiDestroy();