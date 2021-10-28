#pragma once

#include "defines.h"

bool vulkanBackendInit();

void vulkanBackendShutdown();

bool vulkanBeginFrame();

bool vulkanDraw();

void vulkanEndFrame();