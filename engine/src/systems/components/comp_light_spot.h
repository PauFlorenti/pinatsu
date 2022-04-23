#pragma once

#include "comp_base.h"

struct TCompLightSpot : public TCompBase
{
    bool enabled = true;
    glm::vec4 color = glm::vec4(1.0f);
    f32 intensity = 1.0f;
    f32 cosineCutoff;
    f32 spotExponent;
    
    void debugInMenu();
};