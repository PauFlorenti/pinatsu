#include "entitySystem.h"

#include "memory/pmemory.h"
#include "core/logger.h"

static EntitySystem* pState = nullptr;

bool
entitySystemInit(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(EntitySystem);
    if(state == nullptr){
        return true;
    }

    pState = (EntitySystem*)state;
    pState->componentManager.renderCompCount = 0;
    pState->componentManager.transformCompCount = 0;
    pState->componentManager.controllerCompCount = 0;
    pState->componentManager.physicsCompCount = 0;
    pState->componentManager.boxCollisionCompCount = 0;

    pState->entityManager.activeEntitiesCount = 0;

    for(u32 i = 0; i < MAX_ENTITIES_ALLOWED; ++i)
    {
        pState->entityManager.availableEntities[i] = i + 1;
    }

    return true;
}

void
entitySystemShutdown(void* state)
{
    // TODO
}

Entity
entitySystemCreateEntity()
{
    if(pState->entityManager.activeEntitiesCount >= MAX_ENTITIES_ALLOWED) {
        PWARN("Max number of entities reached!");
        return 0;
    }

    for(u32 i = 0; i < MAX_ENTITIES_ALLOWED; ++i)
    {
        if(pState->entityManager.availableEntities[i] != 0){
            Entity id = pState->entityManager.availableEntities[i];
            pState->entityManager.availableEntities[i] = 0;
            pState->entityManager.activeEntitiesCount++;
            return id;
        }
    }

    return 0;
}

void
entitySystemDestroyEntity(Entity entity)
{
    if(entity >= MAX_ENTITIES_ALLOWED || entity < 1 || 
        pState->entityManager.availableEntities[entity] != 0)
    {
        PWARN("The entity received is not valid.");
        return;
    }

    pState->entityManager.availableEntities[entity - 1] = entity;
    pState->entityManager.activeEntitiesCount--;
}

void
entitySystemUpdate(f32 dt)
{
    // Transform component
    

    // Render component

}

void 
entitySystemAddComponent(Entity entity, ComponentType type, void* component)
{
    switch (type)
    {
    case TRANSFORM:
    {   
        pState->componentManager.transformCompCount++;
        TransformComponent* comp = (TransformComponent*)component;
        pState->componentManager.transformComponents[entity] = *comp;
        break;
    }
    case RENDER:
    {
        pState->componentManager.renderCompCount++;
        RenderComponent* comp = (RenderComponent*)component;
        pState->componentManager.renderComponents[entity] = *comp;
        break; 
    }
    case CONTROLLER:
    {
        pState->componentManager.controllerCompCount++;
        ControllerComponent* comp = (ControllerComponent*)component;
        pState->componentManager.controllerComponent[entity] = *comp;
        break;
    }
    case PHYSICS:
    {
        pState->componentManager.physicsCompCount++;
        PhysicsComponent* comp = (PhysicsComponent*)component;
        pState->componentManager.physicsComponent[entity] = *comp;
        break;
    }
    case BOXCOLLIDER:
    {
        pState->componentManager.boxCollisionCompCount++;
        BoxCollisionComponent* comp = (BoxCollisionComponent*)component;
        pState->componentManager.boxCollisionComponent[entity] = *comp;
        break;
    }
    default:
        PWARN("entitySystemAddComponent - Non existant type. No component added.");
        break;
    }
}

void
entitySystemRemoveComponent(Entity entity, ComponentType type)
{
    switch (type)
    {
    case TRANSFORM:
        pState->componentManager.transformComponents[entity] = {};
        pState->componentManager.transformCompCount--;
        break;
    case RENDER:
        pState->componentManager.renderComponents[entity] = {};
        pState->componentManager.renderCompCount--;
        break;
    case CONTROLLER:
        pState->componentManager.controllerComponent[entity] = {};
        pState->componentManager.controllerCompCount--;
        break;
    case PHYSICS:
        pState->componentManager.physicsComponent[entity] = {};
        pState->componentManager.physicsCompCount--;
        break;
    case BOXCOLLIDER:
        pState->componentManager.boxCollisionComponent[entity] = {};
        pState->componentManager.boxCollisionCompCount--;
        break;
    default:
        PWARN("entitySystemRemoveComponent - No available component type found.");
        break;
    }
}

void
entitySystemGetEntities(u32* counter, Entity* entities)
{
    entities = (Entity*)memAllocate(sizeof(Entity) * pState->entityManager.activeEntitiesCount, MEMORY_TAG_ENTITY);

    // TODO check for available entities
}

// TODO Check if entity has such component
void*
entitySystemGetComponent(Entity entity, ComponentType type)
{
    switch (type)
    {
    case TRANSFORM:
        return &pState->componentManager.transformComponents[entity];
        break;
    case RENDER:
        return &pState->componentManager.renderComponents[entity];
        break;
    case CONTROLLER:
        return &pState->componentManager.controllerComponent[entity];
        break;
    case PHYSICS:
        return &pState->componentManager.physicsComponent[entity];
        break;
    case BOXCOLLIDER:
        return &pState->componentManager.boxCollisionComponent[entity];
        break;
    default:
        PWARN("entitySystemGetComponent - No available type found.");
        return nullptr;
        break;
    }
}

void 
entitySystemGetByComponent(u32* counter, ComponentType type, void* outComponents);