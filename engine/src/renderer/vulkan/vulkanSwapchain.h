#pragma once

#include "vulkanTypes.h"

bool vulkanSwapchainCreate(VulkanState* pState);

void vulkanSwapchainDestroy(VulkanState* pState);

bool vulkanSwapchainRecreate(VulkanState* pState);