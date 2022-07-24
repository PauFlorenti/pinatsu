
#include "../comp_base.h"
#include "../comp_camera.h"
#include "../comp_transform.h"

#include "core/input.h"

struct TCompFlyoverController : public TCompBase
{
    DECL_SIBILING_ACCESS();

    f32 speed = 1.0f;
    f32 rotation = 0.5f;
    bool isEnabled = true;
    i32 keyToggleEnable = 0;

public:

    void load(const json& j, const TEntityParseContext& ctx)
    {
        speed = j.value("speed", speed);
        rotation = j.value("rotation", rotation);
        isEnabled = j.value("enabled", isEnabled);
        keyToggleEnable = j.value("keyToggleEnable", keyToggleEnable);
    }

    void debugInMenu()
    {
        ImGui::DragFloat("Speed Factor", &speed, 0.1f, 1.0f, 100.0f);
        ImGui::DragFloat("Rotation Factor", &rotation, 0.001f, 0.001f, 0.1f);
        ImGui::Checkbox("Enabled", &isEnabled);
    }

    void update(f32 deltaTime)
    {
        CEntity* eCamera = getEntityByName("camera");
        PASSERT(eCamera)
        TCompCamera* cCamera = eCamera->get<TCompCamera>();
        TCompTransform* cT = eCamera->get<TCompTransform>();
        PASSERT(cT)

        glm::vec3 fwd   = cT->getForward();
        glm::vec3 right = cT->getRight();
        glm::vec3 up    = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 offset = glm::vec3(0.0f);
        f32 speed = 10.0f;
        if(isKeyDown(KEY_W)) {
            offset = (fwd * speed * deltaTime);
        }
        if(isKeyDown(KEY_S)) {
            offset = (-fwd * speed * deltaTime);
        }
        if(isKeyDown(KEY_UP)) {
            offset = (up * speed * deltaTime);
        }
        if(isKeyDown(KEY_DOWN)) {
            offset = (-up * speed * deltaTime);
        }
        if(isKeyDown(KEY_A)) {
            offset = (right * speed * deltaTime);
        }
        else if(isKeyDown(KEY_D)) {
            offset = (-right * speed * deltaTime);
        }
        if(isMouseButtonDown(RIGHT_MOUSE_BUTTON)) {
            cCamera->locked = true;
        } 
        else {
            cCamera->locked = false;
        }

        f32 yaw, pitch;
        vectorToYawPitch(fwd, &yaw, &pitch);

        if(isKeyDown(KEY_E)) {
            yaw -= 1.0f * deltaTime;
        }

        if(isKeyDown(KEY_Q)) {
            yaw += 1.0f * deltaTime;
        }

        if(cCamera->locked) {
            i32 x, y;
            i32 oldX, oldY;
            getMousePosition(&x, &y);
            getPreviousMousePosition(&oldX, &oldY);
            if(x != oldX || y != oldY) 
            {
                // Rotate
                //glm::vec2 deltaMouse(oldX - x, oldY - y);
                glm::vec2 deltaMouse(x - oldX, y - oldY);
                yaw -= deltaMouse.x * 10.0f * deltaTime;
                pitch += deltaMouse.y * 10.0f * deltaTime;

                // TODO set mouse position to center
                //setMousePosition(appState->m_width / 2, appState->m_height / 2);
            }
        }

        f32 max_pitch = glm::radians(89.95f);
        if (pitch > max_pitch)
            pitch = max_pitch;
        else if (pitch < -max_pitch)
            pitch = -max_pitch;

        fwd = yawPitchToVector(yaw, pitch);
        fwd = glm::normalize(fwd);

        glm::vec3 newPosition = cT->getPosition() + offset;
        cT->lookAt(newPosition, newPosition + fwd, glm::vec3(0.0f, 1.0f, 0.0f));
    }
};

DECL_OBJ_MANAGER("flyover_controller", TCompFlyoverController);