#pragma once
#include "defines.h"

#include "../vulkanTypes.h"

bool 
vulkanCreateForwardShader(
    VulkanState* pState,
    VulkanForwardShader* outShader);

void
vulkanDestroyForwardShader(VulkanState* pState);