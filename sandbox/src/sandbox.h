#pragma once

#include "game.h"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"

typedef struct GameState
{
    f32 deltaTime;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 cameraAxes; // Pitch x, yaw y, roll z

    // this should be given by the entity system.
    u32 nEntities;
    u32 nLights;
} GameState;

// TODO Game status ??
// Active
// MENU
// WIN/LOSE Condition

bool gameInitialize(Game* pGameInst);
bool gameUpdate(Game* pGameInst, f32 deltaTime);
bool gameRender(Game* pGameInst, f32 deltaTime);
void gameOnResize(Game* pGameInst, u32 width, u32 height);