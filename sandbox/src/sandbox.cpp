#include "sandbox.h"

#include "core/input.h"
#include "core/logger.h"
#include "systems/meshSystem.h"
#include "memory/pmemory.h"

// TODO temp
#include "renderer/rendererFrontend.h"

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    //state->view = glm::lookAt(glm::vec3(0.01f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    //state->projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);

    state->view         = glm::mat4(1); //glm::translate(glm::mat4(1), glm::vec3(0, 0, -5)); 
    state->projection   = glm::ortho(
        -(f32)pGameInst->appConfig.startWidth / 2.0f, (f32)pGameInst->appConfig.startWidth / 2.0f, 
        -(f32)pGameInst->appConfig.startHeight / 2.0f, (f32)pGameInst->appConfig.startHeight / 2.0f, -1.0f, 1.0f); 
    state->deltaTime    = 0.0f;
    state->cameraAxes   = glm::vec3(0);

    createMap(state);

    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;

    // TODO make own camera class.

    if(isKeyDown(KEY_A))
    {
        f32 speed = 10.0f * deltaTime;
        glm::vec3 movement = speed * glm::vec3(-1, 0, 0);
        //state->view = glm::translate(state->view, movement);
        state->entities[0].model = glm::translate(state->entities[0].model, movement);
    }
    else if(isKeyDown(KEY_D))
    {
        f32 speed = 10.0f * deltaTime;
        glm::vec3 movement = speed * glm::vec3(1, 0, 0);
        //state->view = glm::translate(state->view, movement);
        state->entities[0].model = glm::translate(state->entities[0].model, movement);
    }
    else if(isKeyDown(KEY_W))
    {
        f32 speed = 10.0f * deltaTime;
        glm::vec3 movement = speed * glm::vec3(0, 1, 0);
        //state->view = glm::translate(state->view, movement);
        state->entities[0].model = glm::translate(state->entities[0].model, movement);
    }
    else if(isKeyDown(KEY_S))
    {
        f32 speed = 10.0f * deltaTime;
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
    RenderMeshData testData{};
    testData.mesh = state->entities[0].mesh;
    testData.model = state->entities[0].model;

    const u32 size = state->nEntities;
    RenderMeshData gameMeshes[5];
    for(u32 i = 0; i < state->nEntities; ++i)
    {
        gameMeshes[i].mesh = state->entities[i].mesh;
        gameMeshes[i].model = state->entities[i].model;
    }

    RenderPacket packet{};
    packet.deltaTime = deltaTime;
    packet.renderMeshDataCount = state->nEntities;
    packet.meshes = gameMeshes;
    renderDrawFrame(packet);

    return true;
}

void gameOnResize(Game* pGameInst, u32 width, u32 height)
{

}

static void createMap(GameState* pGameState)
{
    Mesh* plane = meshSystemGetPlane(1, 1);
    Entity player;
    player.mesh = plane;
    player.model = glm::scale(glm::mat4(1), glm::vec3(100));

    const u32 size = 2;
    u32 nEntities = (size * size) + 1;
    pGameState->nEntities = nEntities;
    pGameState->entities = (Entity*)memAllocate(sizeof(Entity) * nEntities, MEMORY_TAG_GAME);
    pGameState->entities[0] = player;

    Entity map[size][size];
    u32 aux = 1;
    for(u32 y = 0; y < size; ++y)
    {
        for(u32 x = 0; x < size; ++x)
        {
            Entity brick;
            brick.mesh = plane;
            brick.model = glm::translate(glm::mat4(1), glm::vec3(x * 100, y * 100, 0.0f)) * glm::scale(glm::mat4(1), glm::vec3(100));
            map[x][y] = brick;
            pGameState->entities[aux] = map[x][y];
            aux++;
        }
    }
}