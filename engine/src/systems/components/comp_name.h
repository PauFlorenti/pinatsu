#pragma once

#include "comp_base.h"
#include "systems/entity/entity.h"

class TCompName : public TCompBase
{
    DECL_SIBILING_ACCESS();

    static const size_t maxSize = 64;
    char name[maxSize];

public:
    static std::unordered_map<std::string, CHandle> allNames;

    TCompName::TCompName(const char* newName = nullptr);
    const char* getName() {return name;}
    void setName(const char* newName);
    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
};