#include "comp_light_point.h"

#include <external/imgui/imgui.h>

void TCompLightPoint::debugInMenu()
{
    if(ImGui::TreeNode("Light"))
        {
            ImGui::DragFloat3("Colour", &color.r, 1.0f, 0.0f, 1.0f);
            ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f);
            ImGui::DragFloat("Intensity", &intensity, 1.0f, 0.0f);
            ImGui::Checkbox("Enabled", &enabled);
            ImGui::TreePop();
        }
}