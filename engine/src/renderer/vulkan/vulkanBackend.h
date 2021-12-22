#pragma once

#include "defines.h"

#include "external/glm/glm.hpp" // TODO temp

bool vulkanBackendInit(const char* appName);

void vulkanBackendShutdown();

void vulkanBackendOnResize(u32 width, u32 height);

bool vulkanBeginFrame(f32 delta);

bool vulkanDraw();

void vulkanEndFrame();

void vulkanUpdateGlobalState(glm::mat4 view, glm::mat4 projection, f32 dt);