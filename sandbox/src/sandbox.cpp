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

        state->entities[1].model = glm::translate(glm::mat4(1), glm::vec3(b.position, 0.0f));
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

    moveBall(state, *ball, deltaTime);

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

    if(wasKeyDown(KEY_SPACE) && isKeyUp(KEY_SPACE))
    {
        ball->stuck = false;
        ball->velocity = glm::vec2(0.0f, 1.0f) * 100.0f;
        //ball->position = ball->position + ball->velocity * 100.0f * deltaTime;
        //state->entities[1].model = glm::translate(glm::mat4(1), glm::vec3(ball->position, 0.0f));
    }

    static i32 choice = 0;

    if(isKeyUp(KEY_T) && wasKeyDown(KEY_T))
    {
        const char* names[4] = {
            "paving.png",
            "paving2.png",
            "texture.png",
            "cobblestone.png"
        };

        choice++;
        choice %= 4;

        //textureSystemRelease(state->entities[0].mesh->material->diffuseTexture->name);
        state->entities[0].mesh->material->diffuseTexture = textureSystemGet(names[choice]);
    }

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

    const u32 cols = 6;
    const u32 rows = 3;
    u32 map[18] = {
        1, 2, 3, 3, 1, 2,
        2, 3, 1, 1, 2, 3,
        3, 1, 2, 2, 3, 1
    };

    u32 trueHeight = levelHeight / 2;

    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1);
    playerMaterial.type         = MATERIAL_TYPE_FORWARD;

    Material* paddleMat = materialSystemCreateFromData(playerMaterial);
    paddleMat->diffuseTexture = textureSystemGet("paddle.png");

    Material* ballMat = materialSystemCreateFromData(playerMaterial);
    ballMat->diffuseTexture = textureSystemGet("white");

    Entity player;
    player.mesh             = meshSystemGetPlane(64, 16);
    player.mesh->material   = paddleMat;
    player.model            = glm::translate(glm::mat4(1), glm::vec3(0.0f, -300.0f, 0.0f));

    Entity ballEnt;
    ballEnt.mesh = meshSystemGetPlane(8, 8);
    ballEnt.mesh->material = ballMat;
    ballEnt.model = glm::translate(glm::mat4(1), glm::vec3(0.0f, -250.0f, 0.0f));

    u32 nEntities = 20; // 18 + player + ball
    pGameState->nEntities       = nEntities;
    pGameState->entities        = (Entity*)memAllocate(sizeof(Entity) * nEntities, MEMORY_TAG_GAME);
    pGameState->entities[0]     = player;
    pGameState->entities[1]     = ballEnt;

    ball = (Ball*)memAllocate(sizeof(Ball), MEMORY_TAG_ENTITY);
    ball->position = glm::vec3(0.0f, -250.0f, 0.0f);
    ball->stuck = true;

    const u32 w = (f32)levelWidth / (f32)cols;
    const u32 h = (f32)trueHeight / (f32)rows;

    Entity enemies[18];
    for(u32 y = 0; y < rows; y++)
    {
        for(u32 x = 0; x < cols; x++)
        {
            u32 i = y * cols + x;
            MaterialData enemyMaterial{};
            enemyMaterial.type = MATERIAL_TYPE_FORWARD;
            if(map[i] == 1) {
                enemyMaterial.diffuseColor = glm::vec4(1, 0, 0, 1);
                stringCopy("texture.png", enemyMaterial.diffuseTextureName);
            }
            else if(map[i] == 2) {
                enemyMaterial.diffuseColor = glm::vec4(0, 1, 0, 1);
                stringCopy("paving2.png", enemyMaterial.diffuseTextureName);
            }
            else if(map[i] == 3) {
                enemyMaterial.diffuseColor = glm::vec4(0, 0, 1, 1);
                stringCopy("paving.png", enemyMaterial.diffuseTextureName);
            }

            Material* enemyMat = materialSystemCreateFromData(enemyMaterial);
            enemyMat->diffuseTexture = textureSystemGet(enemyMaterial.diffuseTextureName);

            Entity enemy;
            enemy.mesh = meshSystemGetPlane(w, h);
            enemy.mesh->material = enemyMat;
            glm::vec3 pos = glm::vec3(x * w - ((f32)levelWidth / 2.0f - (w / 2)), y * h + (h/2), 0.0f);
            enemy.model = glm::translate(glm::mat4(1), pos);
            // add entities after player and ball.
            pGameState->entities[i + 2] = enemy;
        }
    }
}