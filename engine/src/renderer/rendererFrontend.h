#pragma once

#include "defines.h"

#include "external/glm/glm.hpp" // TODO temp
#include "renderTypes.h"

bool renderSystemInit(u64* memoryRequirement, void* state, const char* appName, void* winHandle);
void renderSystemShutdown(void* state);

bool renderDrawFrame(const RenderPacket& packet);
void renderOnResize(u16 width, u16 height);

bool renderCreateMesh(Mesh* m, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);
bool renderCreateTexture(void* data, Texture* texture);
void renderDestroyTexture(Texture* t);
bool renderCreateMaterial(Material* m);

// TODO temp
void setView(const glm::mat4 view, const glm::mat4 proj);