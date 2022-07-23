#include "sandbox.h"

#include "core/input.h"
#include "core/application.h"

// TODO If application.h is included, does system need to be included too?
#include "systems/resourceSystem.h"
#include "systems/meshSystem.h"
#include "systems/materialSystem.h"
#include "systems/textureSystem.h"
#include "systems/physicsSystem.h"
#include "systems/components/comp_camera.h"
#include "systems/components/comp_transform.h"

// TODO temp
#include "renderer/rendererFrontend.h"
#include "platform/platform.h"

static f32 rot;

// TODO Should it be here ??? probably not ...
#define MAX_ENTITIES_ALLOWED 512

static RenderMeshData gameMeshes[MAX_ENTITIES_ALLOWED];
static LightData gameLight[MAX_ENTITIES_ALLOWED];

#ifdef DEBUG
static bool deferred = true;
#endif

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
    
    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;

    CEntity* eCamera = getEntityByName("camera");
    PASSERT(eCamera)
    TCompCamera* cCamera = eCamera->get<TCompCamera>();
    TCompTransform* cT = eCamera->get<TCompTransform>();
    PASSERT(cCamera)
    
#if DEBUG

    if(wasKeyDown(KEY_F1) && !isKeyDown(KEY_F1)) deferred = !deferred;

    glm::vec3 offset = glm::vec3(0.0f);
    f32 speed = 10.0f;
    if(isKeyDown(KEY_W)) {
        offset = (cCamera->getForward() * speed * deltaTime);
    }
    if(isKeyDown(KEY_S)) {
        offset = (-cCamera->getForward() * speed * deltaTime);
    }
    if(isKeyDown(KEY_UP)) {
        offset = (cCamera->getUp() * speed * deltaTime);
    }
    if(isKeyDown(KEY_DOWN)) {
        offset = (-cCamera->getUp() * speed * deltaTime);
    }
    if(isKeyDown(KEY_A)) {
        offset = (-cCamera->getRight() * speed * deltaTime);
    }
    else if(isKeyDown(KEY_D)) {
        offset = (cCamera->getRight() * speed * deltaTime);
    }
    if(isMouseButtonDown(RIGHT_MOUSE_BUTTON)) {
        cCamera->locked = true;
    } 
    else {
        cCamera->locked = false;
    }

    glm::vec3 front = cT->getForward();
    f32 yaw, pitch;
    vectorToYawPitch(front, &yaw, &pitch);

    if(isKeyDown(KEY_E)) {
        yaw += 1.0f * deltaTime;
    }

    if(isKeyDown(KEY_Q)) {
        yaw -= 1.0f * deltaTime;
    }

    if(cCamera->locked) {
        i32 x, y;
        i32 oldX, oldY;
        getMousePosition(&x, &y);
        getPreviousMousePosition(&oldX, &oldY);
        if(x != oldX || y != oldY) 
        {
            // Rotate
            glm::vec2 deltaMouse(oldX - x, oldY - y);
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

#endif
    front = yawPitchToVector(yaw, pitch);
    front = glm::normalize(front);

    glm::vec3 newPosition = cT->getPosition() + offset;
    cT->lookAt(newPosition, newPosition + front, glm::vec3(0.0f, 1.0f, 0.0f));
    return true;
}

bool gameRender(Game* pGameInst, f32 deltaTime)
{
    GameState* state = (GameState*)pGameInst->state;
    RenderPacket packet{};
    packet.deltaTime = deltaTime;
    
#ifdef DEBUG
    if(!deferred)
        renderDrawFrame(packet);
    else
        renderDeferredFrame(packet);
#else
    renderDrawFrame(packet);
#endif
    return true;
}

void gameOnResize(Game* pGameInst, u32 width, u32 height)
{
    GameState* state = (GameState*)pGameInst->state;
    pGameInst->appConfig.startWidth = width;
    pGameInst->appConfig.startHeight = height;
    f32 ratio           = (f32)width / (f32)height;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
}