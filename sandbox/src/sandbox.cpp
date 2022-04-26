#include "sandbox.h"

#include "core/input.h"
#include "core/logger.h"
#include "core/application.h"
#include "memory/pmemory.h"

// TODO If application.h is included, does system need to be included too?
#include "systems/resourceSystem.h"
#include "systems/meshSystem.h"
#include "systems/materialSystem.h"
#include "systems/textureSystem.h"
#include "systems/physicsSystem.h"
#include "systems/entitySystemComponent.h"
#include "systems/components/comp_transform.h"
#include "systems/components/comp_name.h"
#include "systems/components/comp_light_point.h"
#include "systems/components/comp_parent.h"

// TODO temp
#include "renderer/rendererFrontend.h"
#include "platform/platform.h"

static f32 rot;

static RenderMeshData gameMeshes[MAX_ENTITIES_ALLOWED];
static LightData gameLight[MAX_ENTITIES_ALLOWED];

static Entity camera;

#ifdef DEBUG
static bool deferred = true;
#endif

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);

    ApplicationState* appState = (ApplicationState*)pGameInst->appState;
    EntitySystem* entitySystem = (EntitySystem*)appState->entitySystem;
    
    entitySystem->registerComponent<TCompName>();
    entitySystem->registerComponent<TCompTransform>();
    entitySystem->registerComponent<RenderComponent>();
    entitySystem->registerComponent<TCompLightPoint>();
    entitySystem->registerComponent<CameraComponent>();
    entitySystem->registerComponent<TCompParent>();

    camera = entitySystem->createEntity();
    CameraComponent cameraComp;
    cameraComp.position = glm::vec3(0.0f, 1.0f, 3.0f);
    cameraComp.front    = glm::vec3(0, 0, -1);
    cameraComp.right    = glm::normalize(glm::cross(glm::vec3(0, 1, 0), cameraComp.front));
    cameraComp.up       = glm::normalize(glm::cross(cameraComp.front, cameraComp.right));
    cameraComp.yaw      = -90.0f;
    cameraComp.pitch    = 0.0f;
    cameraComp.locked   = false;

    entitySystem->addComponent(camera, cameraComp);
    entitySystem->addComponent(camera, TCompName("Camera"));
    
/*
    Entity player = entitySystem->createEntity();
    Entity floor = entitySystem->createEntity();
    Entity helmet = entitySystem->createEntity();
    Entity avocado = entitySystem->createEntity();

    TCompTransform t;
    t.position  = glm::vec3(0.0f, 0.0f, -5.0f);
    t.rotation  = glm::quat();
    t.scale     = glm::vec3(1.0);

    TCompTransform t2;
    t2.position  = glm::vec3(0.0f, -2.0f, -5.0f);
    t2.rotation  = glm::quat();
    t2.scale     = glm::vec3(10.0f, 0.25f, 10.0f);

    TCompTransform t3;
    t3.position  = glm::vec3(2.0f, 0.0f, -5.0f);
    t3.rotation  = glm::quat();
    t3.scale     = glm::vec3(1.0);

    TCompTransform t4;
    t4.position  = glm::vec3(0.0f, 0.0f, 0.0f);
    t4.rotation  = glm::quat();
    t4.scale     = glm::vec3(10.0);

    TCompParent parent{};
    parent.addChild(player, floor);

    entitySystem->addComponent(player, t);
    entitySystem->addComponent(player, TCompName("Cube"));
    entitySystem->addComponent(player, parent);
    entitySystem->addComponent(floor, t2);
    entitySystem->addComponent(floor, TCompName("Floor"));
    entitySystem->addComponent(helmet, t3);
    entitySystem->addComponent(helmet, TCompName("Helmet"));
    entitySystem->addComponent(avocado, t4);
    entitySystem->addComponent(avocado, TCompName("Avocado"));

    Resource gltf;
    resourceSystemLoad("cubeMarbre/cube.gltf", RESOURCE_TYPE_GLTF, &gltf);
    Node* cubeNode = (Node*)gltf.data;

    Resource gltfAvocado;
    resourceSystemLoad("Avocado/avocado.gltf", RESOURCE_TYPE_GLTF, &gltfAvocado);
    Node* avocadoNode = (Node*)gltfAvocado.data;

    Resource gltfHelmet;
    resourceSystemLoad("DamagedHelmet/DamagedHelmet.gltf", RESOURCE_TYPE_GLTF, &gltfHelmet);
    Node* helmetNode = (Node*)gltfHelmet.data;

    RenderComponent renderComponent{};
    renderComponent.active      = true;
    renderComponent.material    = cubeNode->material;
    renderComponent.mesh        = cubeNode->mesh;
    entitySystem->addComponent(player, renderComponent);
    entitySystem->addComponent(floor, renderComponent);

    RenderComponent helmetRenderComp{};
    helmetRenderComp.active     = true;
    helmetRenderComp.material   = helmetNode->material;
    helmetRenderComp.mesh       = helmetNode->mesh;
    //entitySystem->addComponent(helmet, helmetRenderComp);

    RenderComponent avocadoRenderComp{};
    avocadoRenderComp.active    = true;
    avocadoRenderComp.material  = avocadoNode->material;
    avocadoRenderComp.mesh      = avocadoNode->mesh;
    //entitySystem->addComponent(avocado, avocadoRenderComp);
*/
    Entity light    = entitySystem->createEntity();
    Entity light1   = entitySystem->createEntity();
    Entity light2   = entitySystem->createEntity();

    TCompLightPoint lightComponent{};
    lightComponent.color        = glm::vec4(1);
    lightComponent.radius       = 30.0f;
    lightComponent.intensity    = 100.0f;

    TCompTransform tLight{};
    tLight.position = glm::vec3(0, 2, 0);

    TCompTransform tLight1{};
    tLight1.position = glm::vec3(7, 2, -9);

    TCompTransform tLight2{};
    tLight2.position = glm::vec3(-7, 2, -7);

    TCompLightPoint lightComp1{};
    lightComp1.enabled = true;
    lightComp1.color        = glm::vec4(1);
    lightComp1.radius       = 40.0f;
    lightComp1.intensity    = 100.0f;

    TCompLightPoint lightComp2{};
    lightComp2.enabled = true;
    lightComp2.color        = glm::vec4(1);
    lightComp2.radius       = 50.0f;
    lightComp2.intensity    = 100.0f;

    entitySystem->addComponent(light, lightComponent);
    entitySystem->addComponent(light, tLight);
    entitySystem->addComponent(light, TCompName("Light"));
    entitySystem->addComponent(light1, lightComp1);
    entitySystem->addComponent(light1, tLight1);
    entitySystem->addComponent(light1, TCompName("Light_01"));
    entitySystem->addComponent(light2, lightComp2);
    entitySystem->addComponent(light2, tLight2);
    entitySystem->addComponent(light2, TCompName("Light_02"));
    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;
    ApplicationState* appState = (ApplicationState*)pGameInst->appState;
    EntitySystem* entitySystem = (EntitySystem*)appState->entitySystem;

    CameraComponent* cameraComp = &entitySystem->getComponent<CameraComponent>(camera);
#if DEBUG

    if(wasKeyDown(KEY_F1) && !isKeyDown(KEY_F1)) deferred = !deferred;

    f32 speed = 10.0f;
    if(isKeyDown(KEY_W)) {
        cameraComp->position = cameraComp->position + (cameraComp->front * speed * deltaTime);
    }
    if(isKeyDown(KEY_S)) {
        cameraComp->position = cameraComp->position - (cameraComp->front * speed * deltaTime);
    }
    if(isKeyDown(KEY_UP)) {
        cameraComp->position = cameraComp->position + (cameraComp->up * speed * deltaTime);
    }
    if(isKeyDown(KEY_DOWN)) {
        cameraComp->position = cameraComp->position - (cameraComp->up * speed * deltaTime);
    }
    if(isKeyDown(KEY_A)) {
        cameraComp->position = cameraComp->position + (cameraComp->right * speed * deltaTime);
    }
    else if(isKeyDown(KEY_D)) {
        cameraComp->position = cameraComp->position - (cameraComp->right * speed * deltaTime);
    }
    if(isMouseButtonDown(RIGHT_MOUSE_BUTTON)) {
        cameraComp->locked = true;
    } 
    else {
        cameraComp->locked = false;
    }

    if(cameraComp->locked) {
        i32 x, y;
        i32 oldX, oldY;
        getMousePosition(&x, &y);
        getPreviousMousePosition(&oldX, &oldY);
        if(x != oldX || y != oldY) {
            // Rotate
            glm::vec2 deltaMouse(oldX - x, oldY - y);
            cameraComp->yaw -= deltaMouse.x * 200 * deltaTime;
            cameraComp->pitch += deltaMouse.y * 200 * deltaTime;

            glm::vec3 front;
            front.x = cos(glm::radians(cameraComp->yaw)) * cos(glm::radians(cameraComp->pitch));
            front.y = sin(glm::radians(cameraComp->pitch));
            front.z = sin(glm::radians(cameraComp->yaw)) * cos(glm::radians(cameraComp->pitch));

            cameraComp->front   = glm::normalize(front);
            cameraComp->right   = glm::normalize(glm::cross(glm::vec3(0, 1, 0), cameraComp->front));
            cameraComp->up      = glm::normalize(glm::cross(cameraComp->front, cameraComp->right));
            // TODO set mouse position to center
            //setMousePosition(appState->m_width / 2, appState->m_height / 2);
        }
    }
#endif

    setView(state->view, state->projection);

    return true;
}

bool gameRender(Game* pGameInst, f32 deltaTime)
{
    GameState* state = (GameState*)pGameInst->state;
    RenderPacket packet{};
    packet.deltaTime = deltaTime;
    
#ifdef DEBUG
    if(!deferred)
        renderDrawFrame(packet);
    else
        renderDeferredFrame(packet);
#else
    renderDrawFrame(packet);
#endif
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