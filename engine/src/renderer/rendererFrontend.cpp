#include "rendererFrontend.h"

#include "rendererBackend.h"

#include "systems/renderSystem.h"
#include "systems/meshSystem.h"
#include "systems/components/comp_transform.h"
#include "systems/components/comp_render.h"

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
    pState->deferredQuad = 0;
    
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
        pState->renderBackend.updateGlobalState((f32)packet.deltaTime);

        for(auto& key : CRenderManager::Get()->keys){
            TCompTransform* cTransform = key.hTransform;
            PASSERT(cTransform)
            RenderMeshData renderData = {cTransform->asMatrix(), key.mesh, key.material};
            pState->renderBackend.drawGeometry(RENDER_PASS_FORWARD, &renderData);
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

        // TODO Update globals ... DeltaTime, ScreenWidth, ScreenHeight, ...

        pState->renderBackend.updateDeferredGlobalState((f32)packet.deltaTime);

        // TODO Get active camera and update its data ...

        /** TODO Deferred renderer ... 
        * - Draw into GBuffers
        * - Draw Decals
        * - Draw AO
        * ...
        */
        CRenderManager::Get()->render();        

        for(auto& key : CRenderManager::Get()->keys){
            TCompTransform* cTransform = key.hTransform;
            PASSERT(cTransform)
            RenderMeshData renderData = {cTransform->asMatrix(), key.mesh, key.material};
            pState->renderBackend.drawGeometry(RENDER_PASS_GEOMETRY, &renderData);
        }
    
        pState->renderBackend.endRenderPass(RENDER_PASS_GEOMETRY);
        pState->renderBackend.submitCommands(RENDER_PASS_GEOMETRY);

        /**
         * TODO
         * Draw light information
         *  - Draw ambient pass
         *  - Draw point lights
         *  - Draw spot lights
         *  - Draw emissives
         *  - Draw Skybox
         */

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