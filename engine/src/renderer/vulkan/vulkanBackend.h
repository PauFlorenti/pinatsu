#pragma once

#include "defines.h"

#include "external/glm/glm.hpp" // TODO temp
#include "resources/resourcesTypes.h" // TODO temp

struct Scene;

bool vulkanBackendInit(const char* appName);
void vulkanBackendShutdown();

void vulkanBackendOnResize(u32 width, u32 height);

bool vulkanBeginFrame(f32 delta);
bool vulkanDraw(const Scene& scene);
void vulkanEndFrame();

bool vulkanCreateMesh(Mesh* mesh, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);
void vulkanUpdateGlobalState(const glm::mat4 view, const glm::mat4 projection, f32 dt);