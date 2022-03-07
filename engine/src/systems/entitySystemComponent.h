#pragma once

#include "defines.h"

#include "core/assert.h"
#include "core/logger.h"
#include "resources/resourcesTypes.h"

#include <external/imgui/imgui.h>
#include <external/glm/glm.hpp>
#include <external/glm/gtc/quaternion.hpp>
#include <external/glm/gtx/matrix_decompose.hpp>

#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <typeinfo>
#include <array>

#define MAX_ENTITIES_ALLOWED 512
#define MAX_COMPONENTS 32

using Entity = u32;
using Signature = std::bitset<MAX_COMPONENTS>;
using ComponentType = u8;

// Transform component
struct TransformComponent
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    glm::mat4 asMatrix() {
        return glm::translate(glm::mat4(1), position)
                * glm::mat4_cast(rotation)
                * glm::scale(glm::mat4(1), scale);
    }

    void fromMatrix(glm::mat4 matrix) {
        glm::decompose(matrix, scale, rotation, position, glm::vec3(0), glm::vec4(0));
        rotation = glm::conjugate(rotation);
    }
};

// Render component
struct RenderComponent
{
    Mesh* mesh;
    Material* material;
    bool active;
};

struct LightPointComponent
{
    glm::vec3 color;
    glm::vec3 position;
    f32 intensity;
    f32 radius;
    bool enabled;

    void renderInMenu()
    {
        if(ImGui::TreeNode("Light"))
        {
            ImGui::DragFloat3("Position ", &position.x, 1.0f);
            ImGui::DragFloat3("Colour", &color.r, 1.0f, 0.0f, 1.0f);
            ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f);
            ImGui::DragFloat("Intensity", &intensity, 1.0f, 0.0f);
            ImGui::TreePop();
        }
    }
};

struct CameraComponent
{
    // Basic attributes
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 front;
    
    // Euler angles
    f32 yaw;
    f32 pitch;

    // Options
    f32 speed;
    f32 zoom;
    bool locked;

    glm::mat4 getView() {
        return glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
    }
    glm::mat4 getProjection(f32 ratio) {
        return glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
    }
};

class EntityManager
{
private:
    // Containing all available IDs.
    std::queue<Entity> availableEntities;
    // Vector holding information of the signature for all of the living entities.
    std::unordered_map<Entity, Signature> entities;
    // Number of created entities.
    u32 livingEntityCount = 0;

public:

    EntityManager()
    {
        // Init for all possible IDs.
        for(Entity entity = 0; entity < MAX_ENTITIES_ALLOWED; ++entity)
        {
            availableEntities.push(entity);
        }
    }

    Entity CreateEntity()
    {
        PASSERT(livingEntityCount < MAX_ENTITIES_ALLOWED);
        Entity id = availableEntities.front();
        availableEntities.pop();
        livingEntityCount++;
        return id;
    }

    void destroyEntity(Entity entity)
    {
        PASSERT(entity < MAX_ENTITIES_ALLOWED);
        PASSERT(entities.find(entity) != entities.end());

        entities.erase(entity);
        availableEntities.push(entity);
        livingEntityCount--;
    }

    void setSignature(Entity entity, Signature signature)
    {
        PASSERT(entity < MAX_ENTITIES_ALLOWED);
        entities[entity] = signature;
    }

    Signature getSignature(Entity entity)
    {
        PASSERT(entity < MAX_ENTITIES_ALLOWED);
        return entities[entity];
    }

    std::unordered_map<Entity, Signature> 
    getAvailableEntities()
    {
        return entities;
    }

};

class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray
{
private:
    // Array holding all available components slots.
    std::array<T, MAX_ENTITIES_ALLOWED> componentArray;
    // Maps to get the component given and entity or the other way around.
    std::unordered_map<Entity, u32> entityToIndex;
    std::unordered_map<u32, Entity> indexToEntity;
    // Total numver of valid entries in the array.
    u32 size;

public:

    void insertData(Entity entity, T component)
    {
        PASSERT(entityToIndex.find(entity) == entityToIndex.end());
        
        // Set the data in the last available spot
        entityToIndex[entity] = size;
        indexToEntity[size] = entity;
        componentArray[size] = component;
        // Increase the size tracker
        size++;
    }

    void removeData(Entity entity)
    {
        PASSERT(entityToIndex.find(entity) != entityToIndex.end());

        // Move last element to the slot being freed.
        u32 removedIdx = entityToIndex[entity];
        u32 lastIdx = size - 1;
        componentArray[removedIdx] = componentArray[lastIdx];
        
        // Store last component to freed slot
        Entity lastEntity = indexToEntity[lastIdx];
        entityToIndex[lastEntity] = removedIdx;
        indexToEntity[removedIdx] = lastEntity;

        entityToIndex.erase(entity);
        indexToEntity.erase(lastIdx);
        // Decrease size tracker.
        size--;
    }

    T& getData(Entity entity)
    {
        PASSERT(entityToIndex.find(entity) != entityToIndex.end());
        return componentArray[entityToIndex[entity]];
    }

    void entityDestroyed(Entity entity) override
    {
        if(entityToIndex.find(entity) != entityToIndex.end())
        {
            removeData(entity);
        }
    }
};

class ComponentManager
{
private:
    // Map a type from a string
    std::unordered_map<std::string, ComponentType> componentTypes;

    // Map a component array from a string
    std::unordered_map<std::string, IComponentArray*> componentArrays;

    ComponentType nextComponentType;

    template<typename T>
    ComponentArray<T>* getComponentArray()
    {
        const char* typeName = typeid(T).name();
        PASSERT(componentTypes.find(typeName) != componentTypes.end());
        return (ComponentArray<T>*)componentArrays[typeName];
    }

public:
    template <typename T>
    void registerComponent()
    {
        const char* typeName = typeid(T).name();
        // Add the component to both maps.
        componentTypes.insert({typeName, nextComponentType});
        componentArrays.insert({typeName, new ComponentArray<T>()});
        // Increment the type so next is different.
        nextComponentType++;
    }

    template <typename T>
    ComponentType getComponentType()
    {
        const char* typeName = typeid(T).name();
        PASSERT(componentTypes.find(typeName) != componentTypes.end());
        return componentTypes[typeName];
    }

    template <typename T>
    void addComponent(Entity entity, T component)
    {
        getComponentArray<T>()->insertData(entity, component);
    }

    template <typename T>
    void removeComponent(Entity entity)
    {
        getComponentArray<T>()->removeData(entity);
    }

    template <typename T>
    T& getComponent(Entity entity)
    {
        return getComponentArray<T>()->getData(entity);
    }

    void entityDestroyed(Entity entity)
    {
        for(auto const& pair : componentArrays)
        {
            pair.second->entityDestroyed(entity);
        }
    }
};

class EntitySystem
{
private:
    ComponentManager* componentManager;
    EntityManager* entityManager;
    static EntitySystem* instance;
public:
    
    static EntitySystem* Get()
    {
        if(!instance){
            instance = new EntitySystem();
        }
        return instance;
    }

    void init()
    {
        instance = this;
        componentManager = new ComponentManager();
        entityManager = new EntityManager();
    }

    Entity createEntity()
    {
        return entityManager->CreateEntity();
    }

    void destroyEntity(Entity entity)
    {
        entityManager->destroyEntity(entity);
        componentManager->entityDestroyed(entity);
    }

    template<typename T>
    void registerComponent()
    {
        componentManager->registerComponent<T>();
    }

    template <typename T>
    void addComponent(Entity entity, T component)
    {
        componentManager->addComponent<T>(entity, component);

        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentType<T>(), true);
        entityManager->setSignature(entity, signature);
    }

    template<typename T>
    void removeComponent(Entity entity)
    {
        componentManager->removeComponent<T>(entity);

        Signature signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponent<T>(entity), false);
        entityManager->setSignature(entity, signature);
    }

    template <typename T>
    T& getComponent(Entity entity)
    {
        return componentManager->getComponent<T>(entity);
    }

    template <typename T>
    ComponentType getComponentType(Entity entity)
    {
        return componentManager->getComponentType<T>();
    }

    std::unordered_map<Entity, Signature>
    getAvailableEntities()
    {
        return entityManager->getAvailableEntities();
    }
};