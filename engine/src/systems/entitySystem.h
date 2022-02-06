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
    CONTROLLER,
    PHYSICS,
    BOXCOLLIDER,
    BRICK,
    LIGHT_POINT,
    LIGHT_SPOT
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

struct BoxCollisionComponent
{
    glm::vec2 min;
    glm::vec2 max;
};

// Physics component
struct PhysicsComponent
{
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 direction;
    f32 mass = 1.0f;
    bool gravity = false;
};

struct LightPointComponent
{
    glm::vec3 color;
    glm::vec3 position;
    f32 intensity;
    f32 radius;
    bool enabled;
};

struct LightSpotComponent
{
    glm::vec3 color;
    f32 intensity;
    bool enabled;
};

struct BrickComponent
{
    bool isSolid = false;
    i8 health = 1;
};

typedef struct ComponentManager
{
    u32 transformCompCount;
    TransformComponent transformComponents[MAX_ENTITIES_ALLOWED];
    u32 renderCompCount;
    RenderComponent renderComponents[MAX_ENTITIES_ALLOWED];
    u32 controllerCompCount;
    ControllerComponent controllerComponent[MAX_ENTITIES_ALLOWED];
    u32 physicsCompCount;
    PhysicsComponent physicsComponent[MAX_ENTITIES_ALLOWED];
    u32 boxCollisionCompCount;
    BoxCollisionComponent boxCollisionComponent[MAX_ENTITIES_ALLOWED];
    u32 brickCompCount;
    BrickComponent* brickComponent[MAX_ENTITIES_ALLOWED];
    u32 lightPointCompCount;
    LightPointComponent lightPointComponent[MAX_ENTITIES_ALLOWED];
    u32 lightSpotCompCount;
    LightSpotComponent lightSpotComponent[MAX_ENTITIES_ALLOWED];
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