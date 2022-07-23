#pragma once

#include "comp_base.h"
#include "systems/entity/entity.h"
#include "renderer/camera.h"

class TCompCamera : public CCamera, public TCompBase
{
    DECL_SIBILING_ACCESS();

public:
    void load(const json& j, TEntityParseContext& ctx);
    void update(f32 dt);
    void debugInMenu();
};