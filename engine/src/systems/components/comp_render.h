#pragma once
#include "comp_base.h"

struct Mesh;
struct Material;

struct TCompRender : public TCompBase
{
    struct TDrawCall
    {
        Mesh* mesh;
        Material* material;
        u32 meshGroup;
        bool active;

        /** Loads the information necessary to create a DrawCall for future rendering */
        bool load(const json& j);
    };

    ~TCompRender();

    void debugInMenu() {};
    void renderDebug() {};
    /** When the entity is created, update the RenderManager with its drawCalls. */
    void onEntityCreated();
    void load(const json& j, TEntityParseContext& ctx);
    
    std::vector<TDrawCall> drawCalls;
    /** Take information from the RenderComponent, clean them from the RenderManager if
     * previously loaded and update them. */
    void updateRenderManager();

private:
    /** Pass this component as handle and clean its DrawCalls from the RenderManager. */
    void cleanFromRenderManager();
};