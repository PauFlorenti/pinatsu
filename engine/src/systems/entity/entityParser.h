#pragma once

#include "systems/components/comp_transform.h"

struct TEntityParseContext 
{
    TEntityParseContext* parent = nullptr;

    bool parsingPrefab = false;

    i32 recursionLevel = 0;

    std::string filename;

    VHandles entitiesLoaded;

    VHandles allEntitiesLoaded;

    CHandle rootEntity;

    CHandle currentEntity;

    TCompTransform rootTransform;

    CHandle findEntityByName(const std::string& name) const;

    TEntityParseContext() = default;
    TEntityParseContext(TEntityParseContext& another, const TCompTransform& deltaTransform);
};

bool parseScene(const std::string& filename, TEntityParseContext& ctx);
CHandle spawn(const std::string& filename, TCompTransform root);