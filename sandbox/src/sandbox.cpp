#include "sandbox.h"

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    state->view         = glm::translate(glm::mat4(1), glm::vec3(2.0f));
    f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
    state->deltaTime    = 0.0f;

    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    return true;
}

bool gameRender(Game* pGameInst, f32 deltaTime)
{
    return true;
}

void gameOnResize(Game* pGameInst, u32 width, u32 height)
{

}