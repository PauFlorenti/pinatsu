#include "sandbox.h"

#include "core/input.h"
#include "core/logger.h"
#include "core/pstring.h"

#include "core/application.h"

// TODO If application.h is included, does system need to be included too?
#include "systems/resourceSystem.h"
#include "systems/meshSystem.h"
#include "systems/materialSystem.h"
#include "systems/textureSystem.h"
//#include "systems/entitySystem.h"
#include "systems/physicsSystem.h"

#include "systems/entitySystemComponent.h"

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

/*
    // Define the scene
    Entity player = entitySystemCreateEntity();
    TransformComponent t{};
    t.position  = glm::vec3(0.0f, 0.0f, 0.0f);
    t.rotation  = glm::quat();
    t.scale     = glm::vec3(1.0f);

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
    Entity light = entitySystemCreateEntity();
    LightPointComponent pointLight{};
    pointLight.color = glm::vec3(0.0f, 1.0f, 0.0f);
    pointLight.enabled = true;
    pointLight.position = glm::vec3(0.0f, 2.0f, 0.0f);
    entitySystemAddComponent(light, LIGHT_POINT, &pointLight);

    Entity lightRed = entitySystemCreateEntity();
    LightPointComponent redPointLight{};
    redPointLight.color = glm::vec3(1.0f, 0.0f, 0.0f);
    redPointLight.enabled = true;
    redPointLight.position = glm::vec3(0.0f, 2.0f, 0.0f);
    entitySystemAddComponent(lightRed, LIGHT_POINT, &redPointLight);

    state->nLights = 2;

    rot = 0.0f;
*/
    ApplicationState* appState = (ApplicationState*)pGameInst->appState;
    EntitySystem* entitySystem = (EntitySystem*)appState->entitySystem;
    
    entitySystem->registerComponent<TransformComponent>();
    entitySystem->registerComponent<RenderComponent>();
    entitySystem->registerComponent<LightPointComponent>();

    Entity player = entitySystem->createEntity();
    Entity floor = entitySystem->createEntity();
    entitySystem->addComponent(player, TransformComponent{glm::vec3(0.0), glm::quat(), glm::vec3(1.0)});
    entitySystem->addComponent(floor, TransformComponent{glm::vec3(0.0f, -2.0f, 0.0f), glm::quat(), glm::vec3(3.0f, 0.5f, 3.0f)});

    Resource gltf;
    resourceSystemLoad("cubeMarbre/cube.gltf", RESOURCE_TYPE_GLTF, &gltf);
    Node* cubeNode = (Node*)gltf.data;

    RenderComponent renderComponent{};
    renderComponent.active = true;
    renderComponent.material = cubeNode->material;
    renderComponent.mesh = cubeNode->mesh;
    entitySystem->addComponent(player, renderComponent);
    entitySystem->addComponent(floor, renderComponent);

    Entity light = entitySystem->createEntity();
    Entity light1 = entitySystem->createEntity();
    Entity light2 = entitySystem->createEntity();

    LightPointComponent lightComponent{};
    lightComponent.color = glm::vec3(1, 0, 0);
    lightComponent.position = glm::vec3(0, 2, 0);
    lightComponent.radius = 1.0f;

    LightPointComponent lightComp1{};
    lightComp1.color = glm::vec3(0, 1, 0);
    lightComp1.position = glm::vec3(3, 2, 0);
    lightComp1.radius = 4.0f;

    LightPointComponent lightComp2{};
    lightComp2.color = glm::vec3(0, 0, 1);
    lightComp2.position = glm::vec3(-3, 2, 0);
    lightComp2.radius = 5.0f;

    entitySystem->addComponent(light, lightComponent);
    entitySystem->addComponent(light1, lightComp1);
    entitySystem->addComponent(light2, lightComp2);

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
        state->view = glm::translate(state->view, glm::vec3(0, -1, 0) * speed * deltaTime);
    }
    if(isKeyDown(KEY_S))
    {
        state->view = glm::translate(state->view, glm::vec3(0, 1, 0) * speed * deltaTime);
    }
    if(isKeyDown(KEY_UP))
    {
        state->view = glm::translate(state->view, glm::vec3(0, 0, 1) * speed * deltaTime);
    }
    if(isKeyDown(KEY_DOWN))
    {
        state->view = glm::translate(state->view, glm::vec3(0, 0, -1) * speed * deltaTime);
    }
    if(isKeyDown(KEY_A)){
        state->view = glm::translate(state->view, glm::vec3(1, 0, 0) * speed * deltaTime);
    }
    else if(isKeyDown(KEY_D)){
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
    RenderPacket packet{};
    packet.deltaTime = deltaTime;
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