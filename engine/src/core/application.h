#pragma once

#include "clock.h"
#include "memory/linearAllocator.h"
#include "systems/modules/module_manager.h"

struct Game;
class EntitySystem;
class CModuleBoot;
class CModuleEntities;

typedef struct ApplicationConfig {
    i16 startPositionX;
    i16 startPositionY;
    i16 startWidth;
    i16 startHeight;
    char* name;
} ApplicationConfig;

typedef struct ApplicationState
{
    Game* pGameInst;
    bool m_isRunning;
    bool m_isSuspended;
    i16 m_width;
    i16 m_height;
    LinearAllocator systemsAllocator;

    Clock clock;
    f64 lastTime;

    u64 memorySystemMemoryRequirements;
    void* memorySystem;

    u64 eventSystemMemoryRequirements;
    void* eventSystem;

    u64 inputSystemMemoryRequirements;
    void* inputSystem;

    u64 platformSystemMemoryRequirements;
    void* platformSystem;

    u64 renderSystemMemoryRequirements;
    void* renderSystem;

    u64 resourceSystemMemoryRequirements;
    void* resourceSystem;

    u64 meshSystemMemoryRequirements;
    void* meshSystem;

    u64 textureSystemMemoryRequirements;
    void* textureSystem;

    u64 materialSystemMemoryRequirements;
    void* materialSystem;

    //u64 entitySystemMemoryRequirements;
    //void* entitySystem;

    u64 physicsSystemMemoryRequirements;
    void* physicsSystem;

    EntitySystem* entitySystem;
    CModuleEntities* entities;
    CModuleBoot* boot;

    CModuleManager* moduleManager;

} ApplicationState;

ApplicationState* appGet();

bool applicationInit(Game* pGameInst);
bool applicationRun();
bool applicationRender(); // TODO receive game instance ??. Internal?
void applicationShutdown();

void 
applicationGetFramebufferSize(u32* width, u32* height);