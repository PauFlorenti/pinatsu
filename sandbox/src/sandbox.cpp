#include "sandbox.h"

#include "core/input.h"
#include "core/logger.h"
#include "systems/meshSystem.h"
#include "systems/materialSystem.h"
#include "memory/pmemory.h"

// TODO temp
#include "renderer/rendererFrontend.h"

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    //state->view = glm::lookAt(glm::vec3(0.01f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    //state->projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);

    state->view         = glm::mat4(1);
    state->projection   = glm::ortho(0.0f, (f32)pGameInst->appConfig.startWidth, 0.0f, (f32)pGameInst->appConfig.startHeight, -1.0f, 1.0f); 
    state->deltaTime    = 0.0f;
    state->cameraAxes   = glm::vec3(0);

    createMap(state, pGameInst->appConfig.startWidth, pGameInst->appConfig.startHeight);

    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;

    // TODO make own camera class.

    f32 vel = 100.0f;
    if(isKeyDown(KEY_A))
    {
        f32 speed = vel * deltaTime;
        glm::vec3 movement = speed * glm::vec3(-1, 0, 0);
        //state->view = glm::translate(state->view, movement);
        state->entities[0].model = glm::translate(state->entities[0].model, movement);
    }
    else if(isKeyDown(KEY_D))
    {
        f32 speed = vel * deltaTime;
        glm::vec3 movement = speed * glm::vec3(1, 0, 0);
        //state->view = glm::translate(state->view, movement);
        state->entities[0].model = glm::translate(state->entities[0].model, movement);
    }
    else if(isKeyDown(KEY_W))
    {
        f32 speed = vel * deltaTime;
        glm::vec3 movement = speed * glm::vec3(0, 1, 0);
        //state->view = glm::translate(state->view, movement);
        state->entities[0].model = glm::translate(state->entities[0].model, movement);
    }
    else if(isKeyDown(KEY_S))
    {
        f32 speed = vel * deltaTime;
        glm::vec3 movement = speed * glm::vec3(0, -1, 0);
        //state->view = glm::translate(state->view, movement);
        state->entities[0].model = glm::translate(state->entities[0].model, movement);
    }

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

    const u32 size = state->nEntities;
    RenderMeshData* gameMeshes = (RenderMeshData*)memAllocate(sizeof(RenderMeshData) * size, MEMORY_TAG_ENTITY);
    for(u32 i = 0; i < size; ++i)
    {
        gameMeshes[i].mesh = state->entities[i].mesh;
        gameMeshes[i].model = state->entities[i].model;
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
    state->projection = glm::ortho(-(f32)width / 2.0f, (f32)width / 2.0f, -(f32)height / 2.0f, (f32)height / 2.0f, -1.0f, 1.0f);
}

static void createMap(GameState* pGameState, u32 levelWidth, u32 levelHeight)
{
    const u32 size = 2;
    u32 w = (levelWidth) / size;
    u32 h = (levelHeight) / size;

    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1, 0, 0, 1);
    playerMaterial.type         = MATERIAL_TYPE_FORWARD;

    MaterialData brickMaterial{};
    brickMaterial.diffuseColor  = glm::vec4(0, 1, 0, 1);
    brickMaterial.type          = MATERIAL_TYPE_FORWARD;

    MaterialData blueMaterial{};
    blueMaterial.diffuseColor   = glm::vec4(0, 0, 1, 1);
    blueMaterial.type           = MATERIAL_TYPE_FORWARD;

    Material* red = materialSystemCreateFromData(playerMaterial);
    Material* green = materialSystemCreateFromData(brickMaterial);
    Material* blue = materialSystemCreateFromData(blueMaterial);

    Entity player;
    player.mesh             = meshSystemGetPlane(100, 50);
    player.mesh->material   = red;
    player.model            = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));

    u32 nEntities = 2;
    pGameState->nEntities       = nEntities;
    pGameState->entities        = (Entity*)memAllocate(sizeof(Entity) * nEntities, MEMORY_TAG_GAME);
    pGameState->entities[0]     = player;

    pGameState->entities[1].mesh            = meshSystemGetTriangle();
    pGameState->entities[1].mesh->material  = blue;
    pGameState->entities[1].model           = glm::scale(glm::translate(glm::mat4(1), glm::vec3(100.0f, 100.0f, 0.0f)), glm::vec3(50));
}