#pragma once

struct VulkanState;

bool
platformCreateVulkanSurface(VulkanState* state);

void
platformSpecificVulkanExtensions(std::vector<const char*>& extensions);