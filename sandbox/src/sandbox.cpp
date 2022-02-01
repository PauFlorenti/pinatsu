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

        //state->entities[1].model = glm::translate(glm::mat4(1), glm::vec3(b.position, 0.0f));
    }
}

void movePlayer(GameState* state)
{
    ControllerComponent* controller = (ControllerComponent*)entitySystemGetComponent(1, CONTROLLER);
    TransformComponent* transform = (TransformComponent*)entitySystemGetComponent(1, TRANSFORM);

    f32 speed = state->deltaTime * controller->velocity;
    if(isKeyDown(KEY_A)) {
        transform->position = transform->position + glm::vec3(1.0f, 0.0f, 0.0f) * speed;
    }
    if(isKeyDown(KEY_D)) {
        transform->position = transform->position + glm::vec3(-1.0f, 0.0f, 0.0f) * speed;
    }
    if(isKeyDown(KEY_W)) {
        transform->position = transform->position + glm::vec3(0.0f, 1.0f, 0.0f) * speed;
    }
    if(isKeyDown(KEY_S)) {
        transform->position = transform->position + glm::vec3(0.0f, -1.0f, 0.0f) * speed;
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

    Entity player = entitySystemCreateEntity();
    TransformComponent t{};
    t.position = glm::vec3(0.0f);
    t.rotation = glm::quat();
    t.scale = glm::vec3(10.0f);

    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1);
    playerMaterial.type = MATERIAL_TYPE_FORWARD;

    Material* paddleMat = materialSystemCreateFromData(playerMaterial);
    paddleMat->diffuseTexture = textureSystemGet("paddle.png");

    RenderComponent r{};
    r.material = paddleMat;
    r.mesh = meshSystemGetPlane(10, 10);

    ControllerComponent controller;
    controller.velocity = 100.0f;
    controller.active = true;

    entitySystemAddComponent(player, TRANSFORM, &t);
    entitySystemAddComponent(player, RENDER, &r);
    entitySystemAddComponent(player, CONTROLLER, &controller);

    TransformComponent t2{};
    t2.position = glm::vec3(10.0f, 10.0f, 0.0f);
    t2.rotation = glm::quat();
    t2.scale = glm::vec3(10.0f);

    Entity enemy = entitySystemCreateEntity();
    entitySystemAddComponent(enemy, TRANSFORM, &t2);
    entitySystemAddComponent(enemy, RENDER, &r);

    state->nEntities = 2;

    //createMap(state, pGameInst->appConfig.startWidth, pGameInst->appConfig.startHeight);

    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;

    // TODO make own camera class.

    movePlayer(state);
    
    static i32 choice = 0;

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
    state->projection = glm::ortho(-(f32)width / 2.0f, (f32)width / 2.0f, -(f32)height / 2.0f, (f32)height / 2.0f, -1.0f, 1.0f);
}

static void createMap(GameState* pGameState, u32 levelWidth, u32 levelHeight)
{
/*
    Entity player = pGameState->ecs.createEntity();

    TransformComp transform;
    transform.position = glm::vec3(0.0);
    transform.rotation = glm::quat();
    transform.scale = glm::vec3(10);
    pGameState->ecs.addComponent(player, transform);

    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1);
    playerMaterial.type = MATERIAL_TYPE_FORWARD;

    Material* paddleMat = materialSystemCreateFromData(playerMaterial);
    paddleMat->diffuseTexture = textureSystemGet("paddle.png");

    RenderComp render;
    render.mesh = meshSystemGetPlane(10, 10);
    render.material = paddleMat;
    pGameState->ecs.addComponent(player, render);

    pGameState->nEntities = 1;
*/

/*
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
    ball->init();

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
    */
}