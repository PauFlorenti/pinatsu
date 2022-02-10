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
        state->drawGeometry = vulkanDrawGeometry;
        state->endRenderPass = vulkanEndRenderPass;
        state->endFrame = vulkanEndFrame;
        state->onResize = vulkanBackendOnResize;
        state->updateGlobalState = vulkanForwardUpdateGlobalState;
        state->onCreateMesh = vulkanCreateMesh;
        state->onCreateTexture = vulkanCreateTexture;
        state->onDestroyTexture = vulkanDestroyTexture;
        state->onCreateMaterial = vulkanCreateMaterial;
        state->drawGui = vulkanImguiRender;

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