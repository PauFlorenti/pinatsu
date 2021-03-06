#pragma once

#include "vulkanTypes.h"

bool readShaderFile(const char* filename, std::vector<char>& buffer);

void vulkanCreateShaderModule(
    const VulkanDevice& device,
    std::vector<char>& buffer,
    VkShaderModule* module);

void vulkanDestroyShaderModule(
    VulkanState& state,
    VulkanShaderObject& module);