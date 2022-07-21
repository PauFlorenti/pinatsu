#include "comp_light_point.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("point_light", TCompLightPoint)

void TCompLightPoint::load(const json& j, TEntityParseContext& ctx)
{
    color       = loadColor(j, "color");
    intensity   = j.value("intensity", intensity);
    radius      = j.value("radius", radius);
    enabled     = j.value("enabled", enabled);
}

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

void TCompLightPoint::renderDebug()
{

}

glm::vec3 TCompLightPoint::getPosition()
{
    TCompTransform* t = get<TCompTransform>();
    PASSERT(t)
    return t->position;
}