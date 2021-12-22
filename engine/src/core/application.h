#pragma once

#include "defines.h"
#include "memory/linearAllocator.h"
#include "clock.h"

struct Game;

typedef struct ApplicationConfig {
    i16 startPositionX;
    i16 startPositionY;
    i16 startWidth;
    i16 startHeight;
    char* name;
} ApplicationConfig;

bool applicationInit(Game* pGameInst);
bool applicationRun();
bool applicationRender(); // TODO receive game instance ??. Internal?
void applicationShutdown();

void applicationGetFramebufferSize(u32* width, u32* height);