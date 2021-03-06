#pragma once

#include "renderer/renderTypes.h"

struct Scene;

bool vulkanBackendInit(const char* appName, void* winHandle);
void vulkanBackendShutdown();

void vulkanBackendOnResize(u32 width, u32 height);

bool vulkanBeginFrame(f32 delta);
void vulkanBeginCommandBuffer(DefaultRenderPasses renderPassid);
bool vulkanBeginRenderPass(DefaultRenderPasses renderPassID);
void vulkanForwardUpdateGlobalState(f32 dt);
void vulkanDeferredUpdateGlobaState(f32 dt);
void vulkanDrawGeometry(DefaultRenderPasses renderPassID, const RenderMeshData* mesh);
void vulkanEndRenderPass(DefaultRenderPasses renderPassID);
void vulkanSubmitCommands(DefaultRenderPasses renderPassID);
void vulkanEndFrame();
void vulkanImguiRender(const RenderPacket& packet);

bool vulkanCreateMesh(Mesh* mesh, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);
void vulkanDestroyMesh(const Mesh* mesh);
bool vulkanCreateTexture(void* data, Texture* texture);
void vulkanDestroyTexture(Texture* texture);
bool vulkanCreateMaterial(Material* m);