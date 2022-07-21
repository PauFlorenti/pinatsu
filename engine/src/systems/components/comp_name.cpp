#include "comp_name.h"

DECL_OBJ_MANAGER("name", TCompName)

std::unordered_map<std::string, CHandle>TCompName::allNames;

TCompName::TCompName(const char* newName)
{
    if(newName)
        strcpy(name, newName);
    else
        strcpy(name, "Default");
}

void TCompName::setName(const char* newName)
{
    strcpy(name, newName);
}

void TCompName::debugInMenu()
{
    ImGui::Text(name);
}

void TCompName::load(const json& j, TEntityParseContext& ctx)
{
    PASSERT(j.is_string())
    setName(j.get<std::string>().c_str());
}

CHandle getEntityByName(const std::string& name)
{
    auto it = TCompName::allNames.find(name.c_str());
    if(it == TCompName::allNames.end())
        return CHandle();
    
    CHandle h = it->second;
    return h.getOwner();
    
    return CHandle();
}