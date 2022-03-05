#pragma once

#include "defines.h"
#include <vector>

struct VulkanState;

bool
platformCreateVulkanSurface(VulkanState* state);

void
platformSpecificVulkanExtensions(std::vector<const char*>& extensions);