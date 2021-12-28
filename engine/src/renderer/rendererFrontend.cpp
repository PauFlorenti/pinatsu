#include "rendererFrontend.h"

#include "rendererBackend.h"
#include "math_types.h"
#include "core/logger.h"

#include "external/glm/gtc/matrix_transform.hpp" // TODO temp

typedef struct RenderFrontendState
{
    RendererBackend renderBackend;
    glm::mat4 view;
    glm::mat4 projection;
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
    if(pState)
    {
        pState->renderBackend.shutdown();
    }
}

bool renderBeginFrame(f64 dt, Mesh* mesh)
{
    if(!pState->renderBackend.beginFrame(dt, mesh)){
        return false;
    }
    pState->renderBackend.updateGlobalState(pState->view, pState->projection, (f32)dt);
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

bool renderCreateMesh(Mesh* m, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices)
{
    return pState->renderBackend.onCreateMesh(m, vertexCount, vertices, indexCount, indices);
}

void setView(const glm::mat4 view, const glm::mat4 proj)
{
    pState->view        = view;
    pState->projection  = proj;
}