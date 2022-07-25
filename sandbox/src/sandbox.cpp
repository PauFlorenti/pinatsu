#include "sandbox.h"

#include "core/input.h"
#include "core/application.h"

// TODO If application.h is included, does system need to be included too?
#include "systems/resourceSystem.h"
#include "systems/meshSystem.h"
#include "systems/materialSystem.h"
#include "systems/textureSystem.h"
#include "systems/physicsSystem.h"
#include "systems/components/comp_camera.h"
#include "systems/components/comp_transform.h"

// TODO temp
#include "renderer/rendererFrontend.h"
#include "platform/platform.h"

// TODO Should it be here ??? probably not ...
#define MAX_ENTITIES_ALLOWED 512

static RenderMeshData gameMeshes[MAX_ENTITIES_ALLOWED];
static LightData gameLight[MAX_ENTITIES_ALLOWED];

#ifdef DEBUG
static bool deferred = false;
#endif

bool gameInitialize(Game* pGameInst)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);

    f32 ratio           = (f32)pGameInst->appConfig.startWidth / (f32)pGameInst->appConfig.startHeight;
    state->projection   = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
    
    return true;
}

bool gameUpdate(Game* pGameInst, f32 deltaTime)
{
    GameState* state = static_cast<GameState*>(pGameInst->state);
    state->deltaTime = deltaTime;
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