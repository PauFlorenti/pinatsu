#pragma once

#include "vulkanTypes.h"

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_win32.h"
#include "external/imgui/imgui_impl_vulkan.h"

void
imguiInit(
    VulkanState* state, 
    const VulkanRenderpass* renderpass);

void
imguiRender(VkCommandBuffer& cmd);

void
imguiDestroy();