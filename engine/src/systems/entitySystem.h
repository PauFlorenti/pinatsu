#pragma once

#include "defines.h"

#include "resources/resourcesTypes.h"

#include "external/glm/glm.hpp"
#include "external/glm/gtc/quaternion.hpp"

#define MAX_ENTITIES_ALLOWED 512

using Entity = u32;
using Signature = u8;

#define MAX_COMPONENTS 32

typedef enum ComponentType
{
    NONE = 0,
    TRANSFORM,
    RENDER,
    CONTROLLER
} ComponentType;

// Transform component
struct TransformComponent
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

// Render component
struct RenderComponent
{
    Mesh* mesh;
    Material* material;
    bool active;
};

// Move component
struct ControllerComponent
{
    f32 velocity;
    bool active;
};

typedef struct ComponentManager
{
    u32 transformCompCount;
    TransformComponent transformComponents[MAX_ENTITIES_ALLOWED];
    u32 renderCompCount;
    RenderComponent renderComponents[MAX_ENTITIES_ALLOWED];
    u32 controllerCompCount;
    ControllerComponent controllerComponent[MAX_ENTITIES_ALLOWED];
} ComponentManager;

// Manager
typedef struct EntityManager
{
    Entity availableEntities[MAX_ENTITIES_ALLOWED];
    Signature signature[MAX_ENTITIES_ALLOWED];
    u32 activeEntitiesCount;

    Entity createEntity();
    void destroyEntity(Entity entity);
    void setSignature(Entity entity, Signature signature);
    Signature getSignature(Entity entity);
} EntityManager;

typedef struct EntitySystem
{
    ComponentManager componentManager;
    EntityManager entityManager;
} EntitySystem;

bool
entitySystemInit(u64* memoryRequirements, void* state);

void
entitySystemShutdown(void* state);

void
entitySystemUpdate(f32 dt);

Entity
entitySystemCreateEntity();

void
entitySystemDestroyEntity(Entity entity);

void 
entitySystemAddComponent(Entity entity, ComponentType type, void* component);

void
entitySystemRemoveComponent(Entity entity, ComponentType type);

void
entitySystemGetEntities(u32* counter, Entity* entities);

void*
entitySystemGetComponent(Entity entity, ComponentType type);

void 
entitySystemGetByComponent(u32* counter, ComponentType type, void* outComponents);