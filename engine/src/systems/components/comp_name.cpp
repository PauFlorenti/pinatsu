
#include "comp_name.h"

#include <external/imgui/imgui.h>

TCompName::TCompName()
{
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