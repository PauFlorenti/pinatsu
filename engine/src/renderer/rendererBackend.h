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
    bool (*beginRenderPass)(const std::string& name);
    void (*drawGeometry)(const RenderMeshData* mesh);
    void (*endRenderPass)(const std::string& renderpass);
    void (*endFrame)();

    void (*bindPipeline)(const std::string& name);
    bool (*activateGlobals)();
    bool (*activateConstants)(const RenderMeshData* mesh);
    bool (*activateInstance)(const RenderMeshData* mesh);
    
    //void (*updateGlobalState)(f32 dt);
    void (*onResize)(u32 width, u32 height);

    bool (*onCreateMesh)(Mesh* m, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);
    bool (*onCreateTexture)(void* data, Texture* texture);
    void (*onDestroyTexture)(Texture* t);
    bool (*onCreateMaterial)(const std::string& pipelineName, Material* m);
    void (*drawGui)();

    void (*activatePipeline)(const std::string& name);


} RendererBackend;

bool rendererBackendInit(RenderBackendAPI api, RendererBackend* state);

void rendererBackendShutdown();