#pragma once

#include "defines.h"

bool vulkanBackendInit(const char* appName);

void vulkanBackendShutdown();

void vulkanBackendOnResize();

bool vulkanBeginFrame();

bool vulkanDraw();

void vulkanEndFrame();