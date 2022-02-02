#include "sandbox.h"

#include "core/input.h"
#include "core/logger.h"
#include "core/pstring.h"
#include "systems/resourceSystem.h"
#include "systems/meshSystem.h"
#include "systems/materialSystem.h"
#include "systems/textureSystem.h"
#include "systems/entitySystem.h"
#include "systems/physicsSystem.h"

#include "memory/pmemory.h"

// TODO temp
#include "renderer/rendererFrontend.h"

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    state->view         = glm::lookAt(glm::vec3(0.01f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);

    state->view         = glm::mat4(1);
    //state->projection   = glm::ortho(0.0f, (f32)pGameInst->appConfig.startWidth, 0.0f, (f32)pGameInst->appConfig.startHeight, -1.0f, 1.0f);
    state->deltaTime    = 0.0f;
    state->cameraAxes   = glm::vec3(0);

    Entity player = entitySystemCreateEntity();
    TransformComponent t{};
    t.position = glm::vec3(0.0f);
    t.rotation = glm::quat();
    t.scale = glm::vec3(100.0f);

    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1);
    playerMaterial.type = MATERIAL_TYPE_FORWARD;

    Material* paddleMat = materialSystemCreateFromData(playerMaterial);
    paddleMat->diffuseTexture = textureSystemGet("paddle.png");

    Resource data;
    resourceSystemLoad("cube.obj", RESOURCE_TYPE_MESH, &data);

    MeshData* mData = (MeshData*)data.data;

    RenderComponent r{};
    r.material = paddleMat;
    r.mesh = meshSystemGetPlane(1, 1);
    r.mesh = meshSystemCreateFromData(mData);

    entitySystemAddComponent(player, TRANSFORM, &t);
    entitySystemAddComponent(player, RENDER, &r);

    state->nEntities = 1;

    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;

#if DEBUG
    if(isKeyDown(KEY_UP))
    {
        state->view = glm::translate(state->view, glm::vec3(0, 1, 0) * 100.0f * deltaTime);
    }
    if(isKeyDown(KEY_DOWN))
    {
        state->view = glm::translate(state->view, glm::vec3(0, -1, 0) * 100.0f * deltaTime);
    }
    if(isKeyDown(KEY_LEFT)){
        state->view = glm::translate(state->view, glm::vec3(-1, 0, 0) * 100.0f * deltaTime);
    }
    else if(isKeyDown(KEY_RIGHT)){
        state->view = glm::translate(state->view, glm::vec3(1, 0, 0) * 100.0f * deltaTime);
    }
#endif

    /*
    No rotation at the moment

    else if(isKeyDown(KEY_Q))
    {
        f32 speed = 1.0f * deltaTime;
        state->cameraAxes.y += speed;
        //state->view = glm::rotate(state->view, glm::radians(state->cameraAxes.y), glm::vec3(0, 1, 0));
    }
    else if(isKeyDown(KEY_E))
    {
        f32 speed = 1.0f * deltaTime;
        state->cameraAxes.y -= speed;
        //state->view = glm::rotate(state->view, glm::radians(state->cameraAxes.y), glm::vec3(0, 1, 0));
    }
    */
    setView(state->view, state->projection);

    return true;
}

bool gameRender(Game* pGameInst, f32 deltaTime)
{
    GameState* state = (GameState*)pGameInst->state;

    RenderComponent* renderComp = nullptr;
    TransformComponent* transComp = nullptr;
    const u32 size = state->nEntities;
    RenderMeshData* gameMeshes = (RenderMeshData*)memAllocate(sizeof(RenderMeshData) * size, MEMORY_TAG_ENTITY);

    for(u32 i = 0; i < size; ++i)
    {
        renderComp = (RenderComponent*)entitySystemGetComponent(i + 1, RENDER);
        transComp = (TransformComponent*)entitySystemGetComponent(i + 1, TRANSFORM);

        gameMeshes[i].mesh = renderComp->mesh;
        gameMeshes[i].material = renderComp->material;
        gameMeshes[i].model = glm::translate(glm::mat4(1), transComp->position) * glm::scale(glm::mat4(1), transComp->scale);
    }

    RenderPacket packet{};
    packet.deltaTime            = deltaTime;
    packet.renderMeshDataCount  = state->nEntities;
    packet.meshes               = gameMeshes;
    renderDrawFrame(packet);

    return true;
}

void gameOnResize(Game* pGameInst, u32 width, u32 height)
{
    GameState* state = (GameState*)pGameInst->state;
    pGameInst->appConfig.startWidth = width;
    pGameInst->appConfig.startHeight = height;
    state->projection = glm::ortho(-(f32)width / 2.0f, (f32)width / 2.0f, -(f32)height / 2.0f, (f32)height / 2.0f, -1.0f, 1.0f);
}