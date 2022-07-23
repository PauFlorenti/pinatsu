#pragma once

#include "renderTypes.h"

typedef enum RenderBackendAPI
{
    VULKAN_API,
    DIRECTX_API,
    OPENGL_API
} RenderBackendAPI;

typedef struct RendererBackend
{
    bool (*init)(const char* appName, void* winHandle);
    void (*shutdown)();
    bool (*beginFrame)(f32 delta);
    void (*beginCommandBuffer)(DefaultRenderPasses renderPass);
    bool (*beginRenderPass)(DefaultRenderPasses renderPass);
    void (*drawGeometry)(DefaultRenderPasses renderPass, const RenderMeshData* mesh);
    void (*endRenderPass)(DefaultRenderPasses renderPass);
    void (*submitCommands)(DefaultRenderPasses renderPass);
    void (*endFrame)();
    void (*onResize)(u32 width, u32 height);
    void (*updateGlobalState)(f32 dt);
    void (*updateDeferredGlobalState)(f32 dt);
    bool (*onCreateMesh)(Mesh* m, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);
    bool (*onCreateTexture)(void* data, Texture* texture);
    void (*onDestroyTexture)(Texture* t);
    bool (*onCreateMaterial)(Material* m);
    void (*drawGui)(const RenderPacket& packet);
} RendererBackend;

bool rendererBackendInit(RenderBackendAPI api, RendererBackend* state);

void rendererBackendShutdown();