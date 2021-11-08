#include "rendererFrontend.h"

#include "rendererBackend.h"
#include "math_types.h"
#include "core/logger.h"

typedef struct RenderFrontendState
{
    RendererBackend renderBackend;
    mat4 view;
    mat4 projection;
    f32 near;
    f32 far;
} RenderFrontendState;

static RenderFrontendState* pState;

bool renderSystemInit(u64* memoryRequirement, void* state, const char* appName)
{
    *memoryRequirement = sizeof(RenderFrontendState);
    if(!state)
        return true;

    pState = static_cast<RenderFrontendState*>(state);
    rendererBackendInit(VULKAN_API, &pState->renderBackend);

    if(!pState->renderBackend.init(appName))
    {
        PFATAL("Render Backend failed to initialize!");
        return false;
    }
    PINFO("Render Backend initialized!");

    return true;

}

void renderSystemShutdown(void* state)
{

}

bool renderBeginFrame(f32 dt)
{
    if(!pState->renderBackend.beginFrame()){
        return false;
    }
    return true;
}

bool renderDrawFrame()
{
    if(!pState->renderBackend.draw()){
        PFATAL("Could not be able to draw.");
        return false;
    }
    return true;
}

void renderEndFrame(f32 dt)
{
    pState->renderBackend.endFrame();
}

void renderOnResize(u16 width, u16 height)
{
    pState->renderBackend.onResize(width, height);
}