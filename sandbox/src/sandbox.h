#pragma once

#include "game.h"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "core/input.h"
#include "core/logger.h"

typedef struct GameState
{
    f32 deltaTime;
    glm::mat4 view;
    glm::mat4 projection;
} GameState;

bool gameInitialize(Game* pGameInst);
bool gameUpdate(Game* pGameInst, f32 deltaTime);
bool gameRender(Game* pGameInst, f32 deltaTime);
void gameOnResize(Game* pGameInst, u32 width, u32 height);