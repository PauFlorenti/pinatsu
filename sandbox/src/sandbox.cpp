#include "sandbox.h"

#include "core/input.h"
#include "core/logger.h"
#include "core/pstring.h"
#include "systems/meshSystem.h"
#include "systems/materialSystem.h"
#include "systems/textureSystem.h"
#include "memory/pmemory.h"

// TODO temp
#include "renderer/rendererFrontend.h"

typedef struct Ball
{
    bool stuck;
    f32 radius;
    glm::vec2 position;
    glm::vec2 velocity;
} Ball;

void moveBall(GameState* state, Ball& b, f32 dt)
{
    if(!b.stuck)
    {
        b.position += b.velocity * dt;

        if(b.position.x <= -400.0f){
            b.velocity.x = -b.velocity.x;
            b.position.x = -400.0f;
        }
        else if(b.position.x >= 400.0f)
        {
            b.velocity.x = -b.velocity.x;
            b.position.x = 400.0f;
        }
        else if(b.position.y <= -400.0f)
        {
            b.velocity.y = -b.velocity.y;
            b.position.y = -400.0f;
        }

        state->entities[state->nEntities - 1].model = glm::translate(state->entities[state->nEntities - 1].model, glm::vec3(b.position, 0.0f));
    }
}

static Ball* ball = nullptr;

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

    //moveBall(state, *ball, deltaTime);

    f32 vel = 200.0f;
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

    if(isKeyDown(KEY_SPACE))
    {
        ball->stuck = false;
        ball->velocity = glm::vec2(0.0f, 1.0f);
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
    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1);
    playerMaterial.type         = MATERIAL_TYPE_FORWARD;

    Material* red = materialSystemCreateFromData(playerMaterial);
    red->diffuseTexture = textureSystemGet("paddle.png"); //textureSystemGetDefaultTexture();

    Entity player;
    player.mesh             = meshSystemGetPlane(100, 25);
    player.mesh->material   = red;
    player.model            = glm::translate(glm::mat4(1), glm::vec3(0.0f, -300.0f, 0.0f));

    u32 nEntities = 1; //(cols * rows) + 2;
    pGameState->nEntities       = nEntities;
    pGameState->entities        = (Entity*)memAllocate(sizeof(Entity) * nEntities, MEMORY_TAG_GAME);
    pGameState->entities[0]     = player;

/*
    const u32 cols = 6;
    const u32 rows = 4;
    u32 w = (levelWidth) / cols;
    u32 h = 130.0f; //(levelHeight) / rows;

    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1, 0, 0, 1);
    playerMaterial.type         = MATERIAL_TYPE_FORWARD;

    MaterialData brickMaterial{};
    brickMaterial.diffuseColor  = glm::vec4(0, 1, 0, 1);
    brickMaterial.type          = MATERIAL_TYPE_FORWARD;

    MaterialData blueMaterial{};
    blueMaterial.diffuseColor   = glm::vec4(0, 0, 1, 1);
    blueMaterial.type           = MATERIAL_TYPE_FORWARD;

    Material* red   = materialSystemCreateFromData(playerMaterial);
    red->diffuseTexture = textureSystemGetDefaultTexture();
    Material* green = materialSystemCreateFromData(brickMaterial);
    Material* blue  = materialSystemCreateFromData(blueMaterial);

    Entity player;
    player.mesh             = meshSystemGetPlane(50, 20);
    player.mesh->material   = red;
    player.model            = glm::translate(glm::mat4(1), glm::vec3(0.0f, -300.0f, 0.0f));

    ball = (Ball*)memAllocate(sizeof(Ball), MEMORY_TAG_ENTITY);
    ball->stuck = true;
    ball->position = glm::vec2(glm::vec3(player.model[3]).x, glm::vec3(player.model[3]).y + 10);

    u32 nEntities = 1;//(cols * rows) + 2;
    pGameState->nEntities       = nEntities;
    pGameState->entities        = (Entity*)memAllocate(sizeof(Entity) * nEntities, MEMORY_TAG_GAME);
    pGameState->entities[0]     = player;

    Mesh* blueBrick         = meshSystemGetPlane(w, h);
    blueBrick->material     = blue;
    stringCopy("BlueBrick", blueBrick->name);
    Mesh* greenBrick        = meshSystemGetPlane(w, h);
    greenBrick->material    = green;
    stringCopy("GreenBrick", greenBrick->name);

    u32 index = 1;
    for(u32 i = 0; i < rows; ++i)
    {
        for(u32 j = 0; j < cols; ++j)
        {
            pGameState->entities[index].mesh = (j%2) == 0 ? blueBrick : greenBrick;
            pGameState->entities[index].model = glm::translate(glm::mat4(1), glm::vec3((j * w) - levelWidth / 2.0f, i * h, 0.0f));
            index++;
        }
    }

    MaterialData ballMat;
    ballMat.diffuseColor = glm::vec4(1);
    ballMat.type = MATERIAL_TYPE_FORWARD;
    stringCopy("Ball Material", ballMat.name);

    Entity ballEntity{};
    ballEntity.model = glm::translate(glm::mat4(1), glm::vec3(ball->position, 0.0f));
    ballEntity.mesh = meshSystemGetPlane(10, 10);
    ballEntity.mesh->material = materialSystemCreateFromData(ballMat);
    pGameState->entities[nEntities - 1] = ballEntity;
    */
}