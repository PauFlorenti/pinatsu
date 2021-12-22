#pragma once

#include "defines.h"

#include "external/glm/glm.hpp" // TODO temp

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
    bool (*draw)();
    void (*endFrame)();
    void (*onResize)(u32 width, u32 height);
    void (*updateGlobalState)(glm::mat4 view, glm::mat4 projection, f32 dt);

} RendererBackend;

bool rendererBackendInit(RenderBackendAPI api, RendererBackend* state);

void rendererBackendShutdown();