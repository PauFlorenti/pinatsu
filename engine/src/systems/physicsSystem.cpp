#include "physicsSystem.h"

#include "core/logger.h"

static PhysicSystem* physicsState = nullptr;

bool 
physicsSystemInit(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(PhysicSystem);
    if(state == nullptr){
        return true;
    }

    physicsState = (PhysicSystem*)state;
    physicsState->rigidBodiesCount = 0;
    for(u32 i = 0; i < MAX_ENTITIES_ALLOWED; i++) {
        physicsState->rigidBodies[i] = 0;
    }
    physicsState->collidersCount = 0;
    for(u32 i = 0; i < MAX_ENTITIES_ALLOWED; i++) {
        physicsState->colliders[i] = 0;
    }
    return true;
}

void
physicsSystemAddEntity(Entity entity)
{
    if(physicsState->rigidBodiesCount >= MAX_ENTITIES_ALLOWED) {
        PWARN("physicsSystemAddEntity - No more room for a new entity");
        return;
    }

    physicsState->rigidBodies[physicsState->rigidBodiesCount] = entity;
    physicsState->rigidBodiesCount++;
}

void
physicsSystemRemoveEntity(Entity entity)
{
    Entity lastEntity = physicsState->rigidBodies[physicsState->rigidBodiesCount - 1];

    if(entity == lastEntity){
        physicsState->rigidBodies[physicsState->rigidBodiesCount - 1] = 0;
        physicsState->rigidBodiesCount--;
        return;
    }

    for(u32 i = 0; i < physicsState->rigidBodiesCount; ++i) {
        if(entity = physicsState->rigidBodies[i]){
            physicsState->rigidBodies[i] = lastEntity;
            physicsState->rigidBodies[physicsState->rigidBodiesCount - 1] = 0;
            physicsState->rigidBodiesCount--;
            return;
        }
    }
}

struct AABB
{
    glm::vec2 min;
    glm::vec2 max;
};

static bool
boxColliderCheckCollision(const AABB& a, const AABB& b)
{
    if(a.max.x < b.min.x || a.min.x > b.max.x) return false;
    if(a.max.y < b.min.y || a.min.y > b.max.y) return false;

    return true;
}

static bool
checkCollision(Entity entity)
{
    TransformComponent* ta = (TransformComponent*)entitySystemGetComponent(entity, TRANSFORM);
    BoxCollisionComponent* a = (BoxCollisionComponent*)entitySystemGetComponent(entity, BOXCOLLIDER);
    for(u32 i = 0; i < physicsState->rigidBodiesCount; ++i) {
        
        if(entity == physicsState->rigidBodies[i])
            continue;
            
        TransformComponent* tb = (TransformComponent*)entitySystemGetComponent(physicsState->rigidBodies[i], TRANSFORM);
        BoxCollisionComponent* b = (BoxCollisionComponent*)entitySystemGetComponent(physicsState->rigidBodies[i], BOXCOLLIDER);

        AABB boxA = {{ta->position.x - a->min.x, ta->position.y - a->min.y},{ta->position.x + a->max.x, ta->position.y + a->max.y}};
        AABB boxB = {{tb->position.x - b->min.x, tb->position.y - b->min.y},{tb->position.x + b->max.x, tb->position.y + b->max.y}};
        return boxColliderCheckCollision(boxA, boxB);
    }
    return false;
}

static void
updateStep(Entity entity, f32 dt)
{
    TransformComponent* t = (TransformComponent*)entitySystemGetComponent(entity, TRANSFORM);
    PhysicsComponent* p = (PhysicsComponent*)entitySystemGetComponent(entity, PHYSICS);
    BoxCollisionComponent* b = (BoxCollisionComponent*)entitySystemGetComponent(entity, BOXCOLLIDER);

    glm::vec3 gravity = glm::vec3(0.0f);
    if(p->gravity) gravity = glm::vec3(0.0f, -9.8f, 0.0f);
    p->acceleration = gravity * p->mass * dt;
    p->velocity += p->acceleration * dt;
    t->position = t->position + p->velocity;
}

void
physicsSystemsUpdate(f32 dt)
{
    for(u32 i = 0; i < physicsState->rigidBodiesCount; ++i) {
        Entity ent = physicsState->rigidBodies[i];
        
        if(checkCollision(ent)) {
            PINFO("Collision detected!");
            return;
        }

        updateStep(ent, dt);
    }
}
