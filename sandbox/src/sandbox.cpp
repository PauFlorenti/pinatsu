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

static f32 rot;

static RenderMeshData gameMeshes[MAX_ENTITIES_ALLOWED];
static LightData gameLight[MAX_ENTITIES_ALLOWED];

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    // TODO make camera an entity
    // Camera definition
    f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
    state->view         = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -10.0f));
    state->deltaTime    = 0.0f;
    state->cameraAxes   = glm::vec3(0);

    // Define the scene
    Entity player = entitySystemCreateEntity();
    TransformComponent t{};
    t.position = glm::vec3(0.0f, 0.0f, 0.0f);
    t.rotation = glm::quat();
    t.scale = glm::vec3(1.0f);

    MaterialData playerMaterial{};
    playerMaterial.diffuseColor = glm::vec4(1);
    playerMaterial.type = MATERIAL_TYPE_FORWARD;

    Resource gltf;
    Material* paddleMat = materialSystemCreateFromData(playerMaterial);
    resourceSystemLoad("cubeMarbre/cube.gltf", RESOURCE_TYPE_GLTF, &gltf);
    Node* cubeNode = (Node*)gltf.data;
    paddleMat->diffuseTexture = textureSystemGet("paving.png");
    resourceSystemLoad("avocado/Avocado.gltf", RESOURCE_TYPE_GLTF, &gltf);
    Node* avocadoNode = (Node*)gltf.data;

    RenderComponent r{};
    r.material = cubeNode->material;
    r.mesh = cubeNode->mesh;

    entitySystemAddComponent(player, TRANSFORM, &t);
    entitySystemAddComponent(player, RENDER, &r);

    Entity plane = entitySystemCreateEntity();
    TransformComponent t1{};
    t1.position = glm::vec3(0.0f, -2.0f, 0.0f);
    t1.rotation = glm::quat_cast(glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(1, 0, 0)));
    t1.scale = glm::vec3(10.0f, 0.2f, 10.0f);
    RenderComponent r1{};
    r1.material = cubeNode->material;
    r1.mesh = cubeNode->mesh;
    entitySystemAddComponent(plane, TRANSFORM, &t1);
    entitySystemAddComponent(plane, RENDER, &r1);

    Entity cube = entitySystemCreateEntity();
    TransformComponent t2{};
    t2.position = glm::vec3(-3.0f, 0.0f, 0.0f);
    t2.rotation = glm::quat();
    t2.scale    = glm::vec3(30.0f);

    RenderComponent r2{};
    r2.material = avocadoNode->material;
    r2.mesh = avocadoNode->mesh;
    entitySystemAddComponent(cube, TRANSFORM, &t2);
    entitySystemAddComponent(cube, RENDER, &r2);

    state->nEntities = 3;

    // Lights
    Entity lightRed = entitySystemCreateEntity();
    LightPointComponent redPointLight{};
    redPointLight.color = glm::vec3(1.0f, 0.0f, 0.0f);
    redPointLight.enabled = true;
    redPointLight.position = glm::vec3(0.0f, 2.0f, -10.0f);
    entitySystemAddComponent(lightRed, LIGHT_POINT, &redPointLight);

    Entity light = entitySystemCreateEntity();
    LightPointComponent pointLight{};
    pointLight.color = glm::vec3(0.0f, 1.0f, 0.0f);
    pointLight.enabled = true;
    pointLight.position = glm::vec3(0.0f, 2.0f, 10.0f);
    entitySystemAddComponent(light, LIGHT_POINT, &pointLight);

    state->nLights = 2;

    rot = 0.0f;

    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;

#if DEBUG
    f32 speed = 10.0f;
    if(isKeyDown(KEY_W))
    {
        state->view = glm::translate(state->view, glm::vec3(0, 0, 1) * speed * deltaTime);
    }
    if(isKeyDown(KEY_S))
    {
        state->view = glm::translate(state->view, glm::vec3(0, 0, -1) * speed * deltaTime);
    }
    if(isKeyDown(KEY_UP))
    {
        state->view = glm::translate(state->view, glm::vec3(0, -1, 0) * speed * deltaTime);
    }
    if(isKeyDown(KEY_DOWN))
    {
        state->view = glm::translate(state->view, glm::vec3(0, 1, 0) * speed * deltaTime);
    }
    if(isKeyDown(KEY_LEFT)){
        state->view = glm::translate(state->view, glm::vec3(1, 0, 0) * speed * deltaTime);
    }
    else if(isKeyDown(KEY_RIGHT)){
        state->view = glm::translate(state->view, glm::vec3(-1, 0, 0) * speed * deltaTime);
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

    // Get Render information of the scene.
    RenderComponent* renderComp = nullptr;
    TransformComponent* transComp = nullptr;
    const u32 size = state->nEntities;

    rot = rot + 10 * deltaTime;
    f32 rotRad = glm::radians(rot);

    for(u32 i = 0; i < 3; ++i)
    {
        renderComp = (RenderComponent*)entitySystemGetComponent(i + 1, RENDER);
        transComp = (TransformComponent*)entitySystemGetComponent(i + 1, TRANSFORM);

        gameMeshes[i].mesh      = renderComp->mesh;
        gameMeshes[i].material  = renderComp->material;
        gameMeshes[i].model     =   glm::translate(glm::mat4(1), transComp->position) * 
                                    //glm::mat4_cast(transComp->rotation) *
                                    glm::rotate(glm::mat4(1), rotRad, glm::vec3(0, 1, 0)) * 
                                    glm::scale(glm::mat4(1), transComp->scale);
    }
    
    // Get light information of the scene.
    LightPointComponent* lightComp;
    for(u32 i = 3; i < 5; i++) 
    {
        lightComp = (LightPointComponent*)entitySystemGetComponent(i + 1, LIGHT_POINT);

        gameLight[i - state->nEntities].color      = lightComp->color;
        gameLight[i - state->nEntities].position   = lightComp->position;
        gameLight[i - state->nEntities].intensity  = lightComp->intensity;
        gameLight[i - state->nEntities].radius     = lightComp->radius;
    }

    RenderPacket packet{};
    packet.deltaTime            = deltaTime;
    packet.renderMeshDataCount  = state->nEntities;
    packet.meshes               = gameMeshes;
    packet.lightDataCount       = state->nLights;
    packet.lights               = gameLight;
    renderDrawFrame(packet);

    return true;
}

void gameOnResize(Game* pGameInst, u32 width, u32 height)
{
    GameState* state = (GameState*)pGameInst->state;
    pGameInst->appConfig.startWidth = width;
    pGameInst->appConfig.startHeight = height;
    f32 ratio           = (f32)width / (f32)height;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
}