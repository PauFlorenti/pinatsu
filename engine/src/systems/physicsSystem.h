#pragma once

#include "defines.h"
#include "entitySystem.h"

struct PhysicSystem
{
    u32 rigidBodiesCount;
    Entity rigidBodies[MAX_ENTITIES_ALLOWED];
    u32 collidersCount;
    Entity colliders[MAX_ENTITIES_ALLOWED];
};

bool 
physicsSystemInit(u64* memoryRequirements, void* state);

void
physicsSystemAddEntity(Entity entity);

void
physicsSystemRemoveEntity(Entity entity);

void
physicsSystemsUpdate(f32 dt);