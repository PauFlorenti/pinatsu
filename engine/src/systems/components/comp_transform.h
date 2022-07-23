#pragma once
#include "comp_base.h"

class TCompTransform : public CTransform, public TCompBase
{
public:
    void debugInMenu();
    void renderDebug();
    void load(const json& j, TEntityParseContext& ctx);
    void set(const CTransform& newT);
};