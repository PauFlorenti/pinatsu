#pragma once

#include "defines.h"

bool vulkanBackendInit(const char* appName);

void vulkanBackendShutdown();

void vulkanBackendOnResize(u32 width, u32 height);

bool vulkanBeginFrame();

bool vulkanDraw();

void vulkanEndFrame();