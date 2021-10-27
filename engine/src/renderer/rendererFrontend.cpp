#include "rendererFrontend.h"

#include "rendererBackend.h"

typedef struct RenderFrontendState
{
    RenderBackend renderBackend;
    // TODO mat4 view
    // TODO mat4 projection
    f32 near;
    f32 far;
} RenderFrontendState;

static RenderFrontendState* pState;

void renderSystemInit(u64* memoryRequirement, void* state, const char* appName)
{
    *memoryRequirement = sizeof(RenderFrontendState);
    if(!state)
        return;

    pState = static_cast<RenderFrontendState*>(state);


}

void renderSystemShutdown(void* state)
{

}

bool renderBeginFrame(f32 dt)
{
    return true;
}

bool renderDrawFrame()
{
    return true;
}

void renderEndFrame(f32 dt)
{

}

void renderOnResize(u16 width, u16 height)
{

}