#include "comp_transform.h"

#include <external/imgui/imgui.h>
#include <external/imgui/ImGuizmo.h>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtx/quaternion.hpp>

// temps
#include "systems/entitySystemComponent.h"
#include "core/application.h"

void 
TCompTransform::debugInMenu()
{
    EntitySystem* entitySystem = EntitySystem::Get();
    auto& entities = entitySystem->getAvailableEntities();

    // TODO Make active camera available somehow. Now it is hardcoded because there is only one camera in the scene.
    CameraComponent* c = &entitySystem->getComponent<CameraComponent>(0);

    if(ImGui::TreeNode("Transform"))
    {
        static ImGuizmo::OPERATION currentOperation(ImGuizmo::TRANSLATE);
        static ImGuizmo::MODE currentMode(ImGuizmo::WORLD);

        ImGui::DragFloat3("Position", &position.x, 1.0f);
        ImGui::DragFloat3("Scale", &scale.x, 1.0f);
        if(ImGui::RadioButton("Translate", currentOperation == ImGuizmo::TRANSLATE))
            currentOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if(ImGui::RadioButton("Rotate", currentOperation == ImGuizmo::ROTATE))
            currentOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if(ImGui::RadioButton("Scale", currentOperation == ImGuizmo::SCALE))
            currentOperation = ImGuizmo::SCALE;
        ImGui::SameLine();
        if(currentOperation != ImGuizmo::SCALE) {
            if(ImGui::RadioButton("Local", currentMode == ImGuizmo::LOCAL))
                currentMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if(ImGui::RadioButton("World", currentMode == ImGuizmo::WORLD))
                currentMode = ImGuizmo::WORLD;
        }

        glm::mat4 matrix = asMatrix();
        glm::mat4 cameraView = c->getView();
        u32 w, h;
        applicationGetFramebufferSize(&w, &h);
        f32 ratio = (f32)w / (f32)h;
        glm::mat4 projection = c->getProjection(ratio);

        ImGui::SameLine();
        if (ImGui::SmallButton("Reset"))
        {
            if(currentOperation == ImGuizmo::TRANSLATE)
                position = glm::vec3(0.0f);
            else if(currentOperation == ImGuizmo::ROTATE)
                rotation = glm::quat();
            else if(currentOperation == ImGuizmo::SCALE)
                scale = glm::vec3(1.0f);
        }

        ImGui::SameLine();
        if(ImGui::SmallButton("All"))
        {
            position = glm::vec3(0.0f);
            rotation = glm::quat();
            scale = glm::vec3(1.0f);
        }

        ImGuizmo::BeginFrame();
        ImGuizmo::Manipulate(
            glm::value_ptr(cameraView), 
            glm::value_ptr(projection), 
            currentOperation, 
            currentMode, 
            glm::value_ptr(matrix));

        if(ImGuizmo::IsUsing())
        {
            f32 translation[3], rotation[3], scale[3];
            ImGuizmo::DecomposeMatrixToComponents(
                glm::value_ptr(matrix),
                translation,
                rotation,
                scale);
            
            fromMatrix(matrix);
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGui::TreePop();
    }
}

void TCompTransform::renderDebug()
{

}