#include "breakout.h"

#include "core/input.h"
#include "core/logger.h"
#include "core/pstring.h"
#include "systems/meshSystem.h"
#include "systems/materialSystem.h"
#include "systems/textureSystem.h"
#include "systems/entitySystem.h"
#include "systems/physicsSystem.h"

#include "memory/pmemory.h"

// TODO temp
#include "renderer/rendererFrontend.h"

static Entity ball;
static Entity player;
static bool stuck;

void movePlayer(GameState* state)
{
    ControllerComponent* controller = (ControllerComponent*)entitySystemGetComponent(player, CONTROLLER);
    TransformComponent* transform = (TransformComponent*)entitySystemGetComponent(player, TRANSFORM);

    f32 speed = state->deltaTime * controller->velocity;
    if(isKeyDown(KEY_A)) {
        transform->position = transform->position + glm::vec3(-1.0f, 0.0f, 0.0f) * speed;
        if(stuck){
            TransformComponent* ballTransform = (TransformComponent*)entitySystemGetComponent(ball, TRANSFORM);
            ballTransform->position = transform->position + glm::vec3(0.0f, 30.0f, 0.0f);
        }
    }
    if(isKeyDown(KEY_D)) {
        transform->position = transform->position + glm::vec3(1.0f, 0.0f, 0.0f) * speed;
        if(stuck){
            TransformComponent* ballTransform = (TransformComponent*)entitySystemGetComponent(ball, TRANSFORM);
            ballTransform->position = transform->position + glm::vec3(0.0f, 30.0f, 0.0f);
        }
    }

    if(wasKeyDown(KEY_SPACE) && isKeyUp(KEY_SPACE)){
        stuck = false;
    }
}

void
ballUpdate(GameState* state, f32 width, f32 height)
{
    if(!stuck)
    {
        TransformComponent* t = (TransformComponent*)entitySystemGetComponent(ball, TRANSFORM);
        PhysicsComponent* p = (PhysicsComponent*)entitySystemGetComponent(ball, PHYSICS);

        t->position += p->velocity * state->deltaTime;

        if(t->position.x <= -width) {
            p->velocity.x = -p->velocity.x;
            t->position.x = -width;
        }
        else if(t->position.x > width)
        {
            p->velocity.x = -p->velocity.x;
            t->position.x = width;
        }
        else if(t->position.y <= -height)
        {
            p->velocity.y = -p->velocity.y;
            t->position.y = -height;
        }
        else if(t->position.y >= height)
        {
            p->velocity.y = -p->velocity.y;
            t->position.y = height;
        }
    }
}

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

    player = entitySystemCreateEntity();
    TransformComponent t{};
    t.position = glm::vec3(0.0f, 100.0f - (f32)pGameInst->appConfig.startHeight / 2.0f, 0.0f);
    t.rotation = glm::quat();
    t.scale = glm::vec3(1.0f);

    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1);
    playerMaterial.type = MATERIAL_TYPE_FORWARD;

    Material* paddleMat = materialSystemCreateFromData(playerMaterial);
    paddleMat->diffuseTexture = textureSystemGet("paddle.png");

    RenderComponent r{};
    r.material = paddleMat;
    r.mesh = meshSystemGetPlane(128, 32);

    BoxCollisionComponent boxcol = {{64, 16}, {64, 16}};

    ControllerComponent controller;
    controller.velocity = 100.0f;
    controller.active = true;

    PhysicsComponent playerPhysics{};
    playerPhysics.gravity = false;

    entitySystemAddComponent(player, TRANSFORM, &t);
    entitySystemAddComponent(player, RENDER, &r);
    entitySystemAddComponent(player, CONTROLLER, &controller);
    entitySystemAddComponent(player, PHYSICS, &playerPhysics);
    entitySystemAddComponent(player, BOXCOLLIDER, &boxcol);

    physicsSystemAddEntity(player);

    TransformComponent t2{};
    t2.position = t.position + glm::vec3(0.0f, 30.0f, 0.0f);
    t2.rotation = glm::quat();
    t2.scale = glm::vec3(1.0f);

    MaterialData ballMaterial{};
    ballMaterial.diffuseColor = glm::vec4(1.0f);
    ballMaterial.type = MATERIAL_TYPE_FORWARD;
    stringCopy("white", ballMaterial.diffuseTextureName);
    
    RenderComponent r2{};
    r2.material = materialSystemCreateFromData(ballMaterial);
    r2.mesh = meshSystemGetPlane(20, 20);

    BoxCollisionComponent ballboxcol = {{10, 10}, {10, 10}};

    PhysicsComponent ballPhysics{};
    ballPhysics.gravity = false;
    ballPhysics.velocity = glm::vec3(100.0f, -350.0f, 0.0f);

    ball = entitySystemCreateEntity();
    entitySystemAddComponent(ball, TRANSFORM, &t2);
    entitySystemAddComponent(ball, RENDER, &r2);
    entitySystemAddComponent(ball, PHYSICS, &ballPhysics);
    entitySystemAddComponent(ball, BOXCOLLIDER, &ballboxcol);

    stuck = true;
    physicsSystemAddEntity(ball);

    createMap(state, pGameInst->appConfig.startWidth, pGameInst->appConfig.startHeight);

    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;

    movePlayer(state);
    ballUpdate(state, (f32)pGameInst->appConfig.startWidth / 2.0f, (f32)pGameInst->appConfig.startHeight / 2.0f);
    
    /*
    for(u32 i = 1; i <= state->nEntities; i++)
    {
        BrickComponent* brick = (BrickComponent*)entitySystemGetComponent(i, BRICK);
        if(brick && brick->health <= 0){
            entitySystemDestroyEntity(i);
        }
    }
    */

    if(!stuck)
        physicsSystemsUpdate(deltaTime);

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

    u32 nEntities = 20; // 18 + player + ball
    pGameState->nEntities       = nEntities;

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

            Entity enemy = entitySystemCreateEntity();

            TransformComponent t{};
            t.position = glm::vec3(x * w - ((f32)levelWidth / 2.0f - (w / 2)), y * h + (h/2), 0.0f);
            t.scale = glm::vec3(1);

            RenderComponent r{};
            r.mesh = meshSystemGetPlane(w, h);
            r.material = enemyMat;

            PhysicsComponent p{};
            p.gravity = false;

            BoxCollisionComponent b{{(f32)w / 2.0f, (f32)h / 2.0f}, {(f32)w / 2.0f, (f32)h / 2.0f}};

            BrickComponent* brick = (BrickComponent*)memAllocate(sizeof(BrickComponent), MEMORY_TAG_ENTITY);
            brick->health = 1;
            brick->isSolid = false;

            entitySystemAddComponent(enemy, TRANSFORM, &t);
            entitySystemAddComponent(enemy, RENDER, &r);
            entitySystemAddComponent(enemy, PHYSICS, &p);
            entitySystemAddComponent(enemy, BOXCOLLIDER, &b);
            entitySystemAddComponent(enemy, BRICK, brick);

            physicsSystemAddEntity(enemy);
        }
    }
}