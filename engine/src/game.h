/**
 *  This header defines the basic structure upon a game has to be built.
 */

#pragma once

#include "defines.h"
#include "core/application.h"

typedef struct Game
{
    void* appState;
    void* state;
    ApplicationConfig appConfig;

    bool (*init)(struct Game* pGameInst);
    bool (*update)(Game* pGameInst, f32 deltaTime);
    bool (*render)(Game* pGameInst, f32 deltaTime);
    void (*onResize)(Game* pGameInsta, u32 width, u32 height);
} Game;