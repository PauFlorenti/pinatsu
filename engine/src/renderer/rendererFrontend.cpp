#include "rendererFrontend.h"

#include "rendererBackend.h"
#include "math_types.h"
#include "core/logger.h"

#include "systems/entitySystemComponent.h"
#include "systems/meshSystem.h"
#include "systems/components/comp_transform.h"

#include "external/glm/gtc/matrix_transform.hpp" // TODO temp

typedef struct RenderFrontendState
{
    RendererBackend renderBackend;
    glm::mat4 view;
    glm::mat4 projection;
    f32 near;
    f32 far;
    Mesh* deferredQuad;
} RenderFrontendState;

static RenderFrontendState* pState;

bool renderSystemInit(u64* memoryRequirement, void* state, const char* appName, void* winHandle)
{
    *memoryRequirement = sizeof(RenderFrontendState);
    if(!state)
        return true;

    pState = static_cast<RenderFrontendState*>(state);
    rendererBackendInit(VULKAN_API, &pState->renderBackend);

    if(!pState->renderBackend.init(appName, winHandle))
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

bool renderDrawFrame(const RenderPacket& packet)
{
    if(pState->renderBackend.beginFrame(packet.deltaTime))
    {
        // Begin command call
        pState->renderBackend.beginCommandBuffer(RENDER_PASS_FORWARD);

        // Begin renderpass
        if(!pState->renderBackend.beginRenderPass(RENDER_PASS_FORWARD))
        {
            PFATAL("renderDrawFrame - Could not begin render pass.");
            return false;
        }

        // Update light descriptor
        pState->renderBackend.updateGlobalState(pState->view, pState->projection, (f32)packet.deltaTime);

        EntitySystem* entitySystem = EntitySystem::Get();
        auto& entities = entitySystem->getAvailableEntities();

        for(auto& entity : entities)
        {
            if(entity.second[entitySystem->getComponentType<RenderComponent>(entity.first)] == 1)
            {
                TransformComponent t = entitySystem->getComponent<TransformComponent>(entity.first);
                RenderComponent r = entitySystem->getComponent<RenderComponent>(entity.first);

                glm::mat4 model = glm::translate(glm::mat4(1), t.position) * glm::mat4_cast(t.rotation) * glm::scale(glm::mat4(1), t.scale);
                RenderMeshData renderData = {model, r.mesh, r.material};
                pState->renderBackend.drawGeometry(RENDER_PASS_FORWARD, &renderData);
            }
        }

        pState->renderBackend.drawGui(packet);
        pState->renderBackend.endRenderPass(RENDER_PASS_FORWARD);
        pState->renderBackend.submitCommands(RENDER_PASS_FORWARD);
        pState->renderBackend.endFrame();

        return true;
    }
    PERROR("Could not start rendering the frame;")
    return false;
}

bool renderDeferredFrame(const RenderPacket& packet)
{
    if(pState->renderBackend.beginFrame(packet.deltaTime))
    {
        // Begin command call
        pState->renderBackend.beginCommandBuffer(RENDER_PASS_GEOMETRY);
        pState->renderBackend.beginRenderPass(RENDER_PASS_GEOMETRY);

        pState->renderBackend.updateDeferredGlobalState(pState->projection, (f32)packet.deltaTime);

        EntitySystem* entitySystem  = EntitySystem::Get();
        auto& entities              = entitySystem->getAvailableEntities();
        
        for(auto& entity : entities)
        {
            if(entity.second[entitySystem->getComponentType<RenderComponent>(entity.first)] == 1)
            {
                TCompTransform t = entitySystem->getComponent<TCompTransform>(entity.first);
                RenderComponent r = entitySystem->getComponent<RenderComponent>(entity.first);

                glm::mat4 model = glm::translate(glm::mat4(1), t.position) * glm::mat4_cast(t.rotation) * glm::scale(glm::mat4(1), t.scale);
                RenderMeshData renderData = {model, r.mesh, r.material};
                pState->renderBackend.drawGeometry(RENDER_PASS_GEOMETRY, &renderData);
            }
        }
    
        pState->renderBackend.endRenderPass(RENDER_PASS_GEOMETRY);
        pState->renderBackend.submitCommands(RENDER_PASS_GEOMETRY);

        // Begin command call
        pState->renderBackend.beginCommandBuffer(RENDER_PASS_DEFERRED);
        pState->renderBackend.beginRenderPass(RENDER_PASS_DEFERRED);
        if(!pState->deferredQuad)
            pState->deferredQuad = meshSystemGetPlane(2, 2);
        RenderMeshData quadData = {glm::mat4(1), pState->deferredQuad, nullptr};
        pState->renderBackend.drawGeometry(RENDER_PASS_DEFERRED, &quadData);
        pState->renderBackend.drawGui(packet);
        pState->renderBackend.endRenderPass(RENDER_PASS_DEFERRED);
        pState->renderBackend.submitCommands(RENDER_PASS_DEFERRED);

        // End frame presents
        pState->renderBackend.endFrame();
        return true;
    }
    return false;
}

void renderOnResize(u16 width, u16 height)
{
    pState->renderBackend.onResize(width, height);
}

bool renderCreateMesh(Mesh* m, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices)
{
    return pState->renderBackend.onCreateMesh(m, vertexCount, vertices, indexCount, indices);
}

bool renderCreateTexture(void* data, Texture* texture)
{
    return pState->renderBackend.onCreateTexture(data, texture);
}

void renderDestroyTexture(Texture* t)
{
    pState->renderBackend.onDestroyTexture(t);
}

bool renderCreateMaterial(Material* m)
{
    return pState->renderBackend.onCreateMaterial(m);
}

void setView(const glm::mat4 view, const glm::mat4 proj)
{
    pState->view        = view;
    pState->projection  = proj;
}