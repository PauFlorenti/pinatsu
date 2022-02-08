#pragma once

#include "defines.h"
#include "vulkanTypes.h"

bool readShaderFile(const char* filename, std::vector<char>& buffer);

void vulkanCreateShaderModule(
    VulkanState* pState,
    std::vector<char>& buffer,
    VkShaderModule* module);

void vulkanDestroyShaderModule(
    VulkanState& state,
    VulkanShaderObject& module);