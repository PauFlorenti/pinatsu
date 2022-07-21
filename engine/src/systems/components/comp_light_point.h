#pragma once

#include "comp_base.h"
#include "systems/entity/entity.h"

struct TCompLightPoint : public TCompBase
{
    DECL_SIBILING_ACCESS();

    glm::vec4 color;
    glm::vec3 position;
    f32 radius      = 1.0f;
    f32 intensity   = 1.0f;
    bool enabled    = true;

    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug(); // TODO

    glm::vec3 getPosition();
};