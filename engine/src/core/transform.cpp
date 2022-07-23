#include "transform.h"

#include "systems/entity/entity.h"
#include "systems/components/comp_camera.h"
#include "systems/components/comp_transform.h"

/**
 * CTransform class functions
 */

glm::vec3 CTransform::getForward() const
{
    return glm::normalize(asMatrix()[2]);
}

glm::vec3 CTransform::getRight() const
{
    return glm::normalize(asMatrix()[0]);
}

glm::vec3 CTransform::getUp() const
{
    return glm::normalize(asMatrix()[1]);
}

glm::mat4 CTransform::asMatrix() const
{
    return glm::translate(glm::mat4(1), position)
            * glm::mat4_cast(rotation)
            * glm::scale(glm::mat4(1), scale);
}

void CTransform::fromMatrix(glm::mat4 matrix)
{
    glm::decompose(
        matrix, 
        scale, 
        rotation, 
        position, 
        glm::vec3(0), 
        glm::vec4(0));
}

CTransform CTransform::combinedWith(const CTransform& deltaTransform) const
{
    CTransform newTransform;
    newTransform.rotation = deltaTransform.rotation * rotation;
    glm::vec3 deltaPosRotated = rotation * deltaTransform.position;
    newTransform.scale = scale * deltaTransform.scale;
    return newTransform;
}

/** Set Euler angles in radians.*/
void CTransform::setEulerAngles(f32 yaw, f32 pitch, f32 roll)
{
    rotation = glm::quat(glm::vec3(pitch, yaw, roll));
}

void CTransform::getEulerAngles(f32* yaw, f32* pitch, f32* roll) const
{
    glm::vec3 fwd = getForward();
    vectorToYawPitch(fwd, yaw, pitch);

    if(!roll)
    {
        glm::vec3 rollZeroLeft = glm::cross(glm::vec3(0, 0, 1), fwd);
        glm::vec3 rollZeroUp = glm::cross(fwd, rollZeroLeft);
        glm::vec3 realLeft = -getRight();
        realLeft = glm::normalize(realLeft);
        rollZeroLeft = glm::normalize(rollZeroLeft);
        rollZeroUp = glm::normalize(rollZeroUp);
        f32 rolledLeftOnUp = glm::dot(realLeft, rollZeroUp);
        f32 rolledLeftOnLeft = glm::dot(realLeft, rollZeroLeft);
        *roll = atan2f(rolledLeftOnUp, rolledLeftOnLeft);
    }
}

void CTransform::lookAt(glm::vec3 eye, glm::vec3 target, glm::vec3 up)
{
    position = eye;
    glm::vec3 fwd = target - eye;
    f32 yaw, pitch;
    vectorToYawPitch(fwd, &yaw, &pitch);
    setEulerAngles(yaw, pitch, 0.f);
}

bool CTransform::fromJson(const json& j)
{
    if(j.count("pos"))
    {
        position = loadVec3(j, "pos");
    }
    if(j.count("lookAt"))
    {
        lookAt(getPosition(), loadVec3(j, "lookAt"), glm::vec3(0, 1, 0));
    }
    if(j.count("rot"))
    {
        rotation = loadQuat(j, "rot");
    }
    if(j.count("euler"))
    {
        glm::vec3 euler = loadVec3(j, "euler");
        euler.x = glm::radians(euler.x);
        euler.y = glm::radians(euler.y);
        euler.z = glm::radians(euler.z);
        rotation = glm::quat(euler);
    }
    if(j.count("scale"))
    {
        const json& jscale = j["scale"];
        if(jscale.is_number())
        {
            f32 fscale = jscale.get<f32>();
            scale = glm::vec3(fscale);
        }
        else {
            scale = loadVec3(j, "scale");
        }
    }
    return true;
}

bool CTransform::renderInMenu()
{
    bool changed = false;

    changed |= renderGuizmo();
    return changed;
}

bool CTransform::renderGuizmo()
{
    // TODO Make active camera available somehow. Now it is hardcoded because there is only one camera in the scene.

    // TODO should get the active camera.
    CEntity* eCamera = getEntityByName("camera");
    TCompCamera* c = eCamera->get<TCompCamera>();
    TCompTransform* t = eCamera->get<TCompTransform>();

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
    //u32 w, h;
    //applicationGetFramebufferSize(&w, &h);
    //f32 ratio = (f32)w / (f32)h;
    glm::mat4 projection = c->getProjection(/* ratio */);
    bool changed = false;

    ImGui::SameLine();
    if (ImGui::SmallButton("Reset"))
    {
        if(currentOperation == ImGuizmo::TRANSLATE)
            position = glm::vec3(0.0f);
        else if(currentOperation == ImGuizmo::ROTATE)
            rotation = glm::quat();
        else if(currentOperation == ImGuizmo::SCALE)
            scale = glm::vec3(1.0f);
        changed = true;
    }

    ImGui::SameLine();
    if(ImGui::SmallButton("All"))
    {
        position = glm::vec3(0.0f);
        rotation = glm::quat();
        scale = glm::vec3(1.0f);
        changed = true;
    }

    ImGuizmo::BeginFrame();
    changed |= ImGuizmo::Manipulate(
        glm::value_ptr(cameraView), 
        glm::value_ptr(projection), 
        currentOperation, 
        currentMode, 
        glm::value_ptr(matrix));

    /*if(ImGuizmo::IsUsing())
    {
        f32 translation[3], rotation[3], scale[3];
        ImGuizmo::DecomposeMatrixToComponents(
            glm::value_ptr(matrix),
            translation,
            rotation,
            scale);
        
        fromMatrix(matrix);
    }*/

    if(changed){
        fromMatrix(matrix);
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    return changed;
}
