#include "comp_render.h"

#include "systems/resourceSystem.h"
#include "systems/renderSystem.h"

DECL_OBJ_MANAGER("render", TCompRender);

TCompRender::~TCompRender()
{
}

bool TCompRender::TDrawCall::load(const json& j)
{
    Resource gltf;
    std::string name = j["mesh"];
    resourceSystemLoad(name.c_str(), RESOURCE_TYPE_GLTF, &gltf);
    Node* n     = (Node*)gltf.data;
    mesh        = n->mesh;
    material    = n->material;
    meshGroup   = j.value("meshGroup", 0);
    active      = j.value("enabled", active);
    return true;
}

void TCompRender::onEntityCreated()
{
    updateRenderManager();
}

void TCompRender::load(const json& j, TEntityParseContext& ctx)
{
    if(j.is_array())
    {
        for(auto& item : j.items())
        {
            const json& jentry = item.value();
            TDrawCall dc;
            if(dc.load(jentry)){
                drawCalls.push_back(dc);
            }
        }
    }
}

void TCompRender::updateRenderManager()
{
    cleanFromRenderManager();

    CHandle handle(this);
    for(auto& dc : drawCalls)
    {
        if(dc.active == false)
            continue;
        
        CRenderManager::Get()->addKey(
            handle,
            dc.mesh,
            dc.material
        );
    }
}

void TCompRender::cleanFromRenderManager()
{
    CHandle handle(this);
    // TODO: RenderManager should clean all DrawCalls from a given component.
    //CRenderManager::Get()
}