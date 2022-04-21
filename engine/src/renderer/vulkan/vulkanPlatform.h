#pragma once

#include "defines.h"

struct VulkanState;

bool
platformCreateVulkanSurface(VulkanState* state);

void
platformSpecificVulkanExtensions(std::vector<const char*>& extensions);