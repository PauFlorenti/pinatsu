#pragma once

#include "defines.h"

#include "external/glm/glm.hpp" // TODO temp
#include "renderer/renderTypes.h"

struct Scene;

bool vulkanBackendInit(const char* appName);
void vulkanBackendShutdown();

void vulkanBackendOnResize(u32 width, u32 height);

bool vulkanBeginFrame(f32 delta);
bool vulkanBeginRenderPass(DefaultRenderPasses renderPassID);
void vulkanForwardUpdateGlobalState(const glm::mat4 view, const glm::mat4 projection, f32 dt);
void vulkanDrawGeometry(const RenderMeshData* mesh);
void vulkanEndRenderPass(DefaultRenderPasses renderPassID);
void vulkanEndFrame();

bool vulkanCreateMesh(Mesh* mesh, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);
void vulkanDestroyMesh(const Mesh* mesh);
//bool vulkanCreateTexture(void* data, Texture* texture);
//void vulkanDestroyTexture(const Texture* texture);
bool vulkanCreateMaterial(Material* m);