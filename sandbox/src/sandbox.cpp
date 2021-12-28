#include "sandbox.h"

#include "core/input.h"
#include "core/logger.h"

// TODO temp
#include "renderer/rendererFrontend.h"

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    state->view         = glm::lookAt(glm::vec3(0.01f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
    state->deltaTime    = 0.0f;
    state->cameraAxes   = glm::vec3(0);

    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;
    if(isKeyDown(KEY_A))
    {
        f32 speed = 10.0f * deltaTime;
        glm::vec3 movement = speed * glm::vec3(-1, 0, 0);
        state->view = glm::translate(state->view, movement);
    }
    else if(isKeyDown(KEY_D))
    {
        f32 speed = 10.0f * deltaTime;
        glm::vec3 movement = speed * glm::vec3(1, 0, 0);
        state->view = glm::translate(state->view, movement);
    }
    else if(isKeyDown(KEY_W))
    {
        f32 speed = 10.0f * deltaTime;
        glm::vec3 movement = speed * glm::vec3(0, 1, 0);
        state->view = glm::translate(state->view, movement);
    }
    else if(isKeyDown(KEY_S))
    {
        f32 speed = 10.0f * deltaTime;
        glm::vec3 movement = speed * glm::vec3(0, -1, 0);
        state->view = glm::translate(state->view, movement);
    }
    else if(isKeyDown(KEY_Q))
    {
        f32 speed = 1.0f * deltaTime;
        state->cameraAxes.y += speed;
        state->view = glm::rotate(state->view, glm::radians(state->cameraAxes.y), glm::vec3(0, 1, 0));
    }
    else if(isKeyDown(KEY_E))
    {
        f32 speed = 1.0f * deltaTime;
        state->cameraAxes.y -= speed;
        state->view = glm::rotate(state->view, glm::radians(state->cameraAxes.y), glm::vec3(0, 1, 0));
    }
    setView(state->view, state->projection);

    return true;
}

bool gameRender(Game* pGameInst, f32 deltaTime)
{
    return true;
}

void gameOnResize(Game* pGameInst, u32 width, u32 height)
{

}