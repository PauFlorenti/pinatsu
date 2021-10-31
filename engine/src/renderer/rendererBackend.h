#pragma once

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

    bool (*beginFrame)();

    bool (*draw)();

    void (*endFrame)();

    void (*onResize)();

} RendererBackend;

bool rendererBackendInit(RenderBackendAPI api, RendererBackend* state);

void rendererBackendShutdown();