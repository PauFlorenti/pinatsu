#pragma once

typedef struct RenderBackend
{
    bool (*init)();

    void (*shutdown)();

} RenderBackend;

bool rendererBackendInit();

void rendererBackendShutdown();