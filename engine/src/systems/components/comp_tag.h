#pragma once

#include "comp_base.h"

struct TCompTag : public TCompBase
{
    const static u32 maxTags = 4;
    u32 tags[maxTags];

    void load(const json& j, TEntityParseContext& ctx);
};