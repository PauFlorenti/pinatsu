#pragma once

#include "defines.h"

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

    bool (*beginFrame)(f64 delta);

    bool (*draw)();

    void (*endFrame)();

    void (*onResize)(u32 width, u32 height);

} RendererBackend;

bool rendererBackendInit(RenderBackendAPI api, RendererBackend* state);

void rendererBackendShutdown();