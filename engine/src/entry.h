#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game.h"

extern bool createGame(Game* pGameInst);

int main()
{
    Game gameInstance;
    if(!createGame(&gameInstance))
    {
        PFATAL("Could not create game instance! Shutting down.");
        return -1;
    }

    if(!gameInstance.init || !gameInstance.update || !gameInstance.render || !gameInstance.onResize)
    {
        PFATAL("Game's function pointers are not initialized! Shutting down.");
        return -2;
    }

    // Init game
    if(!applicationInit(&gameInstance))
    {
        PFATAL("Application failed to initialize. Shutting down.");
        return 1;
    }

    if(!applicationRun())
    {
        PFATAL("Application did not shut down as expected.");
        return 2;
    }

    return 0;
}