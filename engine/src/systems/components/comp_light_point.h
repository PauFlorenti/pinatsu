#pragma once

#include "comp_base.h"

struct TCompLightPoint : public TCompBase
{
    glm::vec4 color;
    f32 intensity = 1.0f;
    f32 radius = 1.0f;
    bool enabled = true;

    void debugInMenu();
};