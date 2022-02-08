#pragma once

#include "vulkanTypes.h"

bool 
pickPhysicalDevice(VulkanState* state);

bool 
createLogicalDevice(VulkanState* state);

void 
destroyLogicalDevice(VulkanState& state);

void
vulkanDeviceGetDepthFormat(VulkanState& device);