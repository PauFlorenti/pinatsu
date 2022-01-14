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
        state->beginRenderPass = vulkanBeginRenderPass;
        state->draw = vulkanDraw;
        state->endRenderPass = vulkanEndRenderPass;
        state->endFrame = vulkanEndFrame;
        state->onResize = vulkanBackendOnResize;
        state->updateGlobalState = vulkanUpdateGlobalState;
        state->onCreateMesh = vulkanCreateMesh;
        state->onCreateTexture = vulkanCreateTexture;

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