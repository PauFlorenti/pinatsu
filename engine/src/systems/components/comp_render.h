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

        bool load(const json& j);
    };

    ~TCompRender();

    void debugInMenu() {};
    void renderDebug() {};
    void onEntityCreated();
    void load(const json& j, TEntityParseContext& ctx);
    
    std::vector<TDrawCall> drawCalls;
};