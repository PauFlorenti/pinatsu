#include "entry.h"

#include "game.h"
#include "breakout.h"
#include "memory/pmemory.h"

bool createGame(Game* game)
{
    game->appConfig.startPositionX  = 100;
    game->appConfig.startPositionY  = 100;
    game->appConfig.startWidth      = 1200;
    game->appConfig.startHeight     = 800;
    game->appConfig.name            = "Breakout Game";

    game->init      = gameInitialize;
    game->update    = gameUpdate;
    game->render    = gameRender;
    game->onResize  = gameOnResize;

    game->appState  = nullptr;
    game->state     = memAllocate(sizeof(GameState), MEMORY_TAG_GAME);

    return true;
}