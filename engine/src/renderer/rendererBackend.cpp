#include "rendererBackend.h"
#include "renderer/vulkan/vulkanBackend.h"

bool rendererBackendInit(RenderBackendAPI api, RendererBackend* state)
{
    switch (api)
    {
    case VULKAN_API :
    {
        state->init = vulkanBackendInit;

        state->shutdown = vulkanBackendShutdown;

        state->beginFrame = vulkanBeginFrame;

        state->draw = vulkanDraw;

        state->endFrame = vulkanEndFrame;

        state->onResize = vulkanBackendOnResize;

        return true;
    }
        break;
    
    default:
        break;
    }

    return false;
}

void rendererBackendShutdown()
{

}