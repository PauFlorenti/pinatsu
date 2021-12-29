#pragma once

#include "defines.h"

#include "external/glm/glm.hpp" // TODO temp
#include "resources/resourcesTypes.h" // TODO temp
#include "scene/scene.h"

bool renderSystemInit(u64* memoryRequirement, void* state, const char* appName);
void renderSystemShutdown(void* state);

bool renderBeginFrame(f64 dt);
bool renderDrawFrame(const Scene& scene);
void renderEndFrame(f32 dt);

void renderOnResize(u16 width, u16 height);

bool renderCreateMesh(Mesh* m, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);

// TODO temp
void setView(const glm::mat4 view, const glm::mat4 proj);