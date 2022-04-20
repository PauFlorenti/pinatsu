#include "comp_name.h"
#include "systems/handle/handle.h"

#include <external/imgui/imgui.h>

DECL_OBJ_MANAGER("name", TCompName)

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