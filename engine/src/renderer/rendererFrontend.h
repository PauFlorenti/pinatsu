#pragma once

#include "defines.h"

#include "external/glm/glm.hpp" // TODO temp

bool renderSystemInit(u64* memoryRequirement, void* state, const char* appName);

void renderSystemShutdown(void* state);

bool renderBeginFrame(f64 dt);

bool renderDrawFrame();

void renderEndFrame(f32 dt);

void renderOnResize(u16 width, u16 height);

// TODO temp
void setView(glm::mat4 view, glm::mat4 proj);