#pragma once

#include "defines.h"

#include "external/glm/glm.hpp" // TODO temp
#include "renderTypes.h"

typedef enum RenderBackendAPI
{
    VULKAN_API,
    DIRECTX_API,
    OPENGL_API
} RenderBackendAPI;

typedef struct RendererBackend
{
    bool (*init)(const char* appName);
    void (*shutdown)();
    bool (*beginFrame)(f32 delta);
    bool (*beginRenderPass)(DefaultRenderPasses renderPass);
    void (*drawGeometry)(const RenderMeshData* mesh);
    void (*endRenderPass)(DefaultRenderPasses renderPass);
    void (*endFrame)();
    void (*onResize)(u32 width, u32 height);
    void (*updateGlobalState)(glm::mat4 view, glm::mat4 projection, f32 dt);
    bool (*onCreateMesh)(Mesh* m, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);
    bool (*onCreateTexture)(void* data, Texture* texture, Resource* t);
    void (*onDestroyTexture)(Texture* t);
    bool (*onCreateMaterial)(Material* m);

} RendererBackend;

bool rendererBackendInit(RenderBackendAPI api, RendererBackend* state);

void rendererBackendShutdown();