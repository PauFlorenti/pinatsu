
#include "application.h"

#include "platform/platform.h"
#include "memory/pmemory.h"

#include "event.h"
#include "input.h"
#include "logger.h"
#include "assert.h"
#include "game.h"

#include "renderer/rendererFrontend.h"

#include "systems/resourceSystem.h"
#include "systems/meshSystem.h"
#include "systems/textureSystem.h"
#include "systems/materialSystem.h"

// TODO temp
#include "scene/scene.h"

typedef struct ApplicationState
{
    Game* pGameInst;
    bool m_isRunning;
    bool m_isSuspended;
    i16 m_width;
    i16 m_height;
    LinearAllocator systemsAllocator;

    Clock clock;
    f64 lastTime;

    Scene* scene;

    u64 memorySystemMemoryRequirements;
    void* memorySystem;

    u64 eventSystemMemoryRequirements;
    void* eventSystem;

    u64 inputSystemMemoryRequirements;
    void* inputSystem;

    u64 platformSystemMemoryRequirements;
    void* platformSystem;

    u64 renderSystemMemoryRequirements;
    void* renderSystem;

    u64 resourceSystemMemoryRequirements;
    void* resourceSystem;

    u64 meshSystemMemoryRequirements;
    void* meshSystem;

    u64 textureSystemMemoryRequirements;
    void* textureSystem;

    u64 materialSystemMemoryRequirements;
    void* materialSystem;
} ApplicationState;

static ApplicationState* pState;

bool appOnEvent(u16 code, void* sender, void* listener, eventContext data);
bool appOnResize(u16 code, void* sender, void* listener, eventContext data);
bool appOnKey(u16 code, void* sender, void* listener, eventContext data);

/**
 * Initialize all systems needed for the app to run.
 */
bool applicationInit(Game* pGameInst)
{
    if(pGameInst->appState != nullptr)
    {
        PERROR("Game application State called twice! That is not the expected behaviour.");
        return false;
    }

    pGameInst->appState = memAllocate(sizeof(ApplicationState), MEMORY_TAG_APPLICATION);
    pState = static_cast<ApplicationState*>(pGameInst->appState);
    pState->pGameInst       = pGameInst;
    pState->m_isRunning     = true;
    pState->m_isSuspended   = false;

    pState->m_width = pState->pGameInst->appConfig.startWidth;
    pState->m_height = pState->pGameInst->appConfig.startHeight;

    u64 systemsAllocatorTotalSize = 64 * 1024 * 1024; // 64mb
    linearAllocatorCreate(systemsAllocatorTotalSize, 0, &pState->systemsAllocator);

    //* Init Subsystems
    // Init memory system
    memorySystemInit(&pState->memorySystemMemoryRequirements, nullptr);
    pState->memorySystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->memorySystemMemoryRequirements);
    memorySystemInit(&pState->memorySystemMemoryRequirements, pState->memorySystem);

    // Init event system.
    eventSystemInit(&pState->eventSystemMemoryRequirements, nullptr);
    pState->eventSystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->eventSystemMemoryRequirements);
    eventSystemInit(&pState->eventSystemMemoryRequirements, pState->eventSystem);

    // Init input system.
    inputSystemInit(&pState->inputSystemMemoryRequirements, nullptr);
    pState->inputSystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->inputSystemMemoryRequirements);
    inputSystemInit(&pState->inputSystemMemoryRequirements, pState->inputSystem);
    eventRegister(EVENT_CODE_BUTTON_PRESSED, 0, appOnKey);
    eventRegister(EVENT_CODE_BUTTON_RELEASED, 0, appOnKey);

    // Init platform system.
    platformStartup(&pState->platformSystemMemoryRequirements, 0, 0, 0, 0, 0, 0);
    pState->platformSystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->platformSystemMemoryRequirements);
    if(!platformStartup(&pState->platformSystemMemoryRequirements, 
        pState->platformSystem, "Pinatsu platform", 100, 100, pState->m_width, pState->m_height))
    {
        PFATAL("Platform system could not be initialized!");
        return false;
    }

    eventRegister(EVENT_CODE_APP_QUIT, 0, appOnEvent);
    eventRegister(EVENT_CODE_RESIZED, 0, appOnResize);

    // Init renderer system.
    renderSystemInit(&pState->renderSystemMemoryRequirements, nullptr, nullptr);
    pState->renderSystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->renderSystemMemoryRequirements);
    if(!renderSystemInit(&pState->renderSystemMemoryRequirements, pState->renderSystem, "Pinatsu engine"))
    {
        PFATAL("Render system could not be initialized! Shuting down now.");
        return false;
    }
    PINFO(getMemoryUsageStr().c_str());

    // Init resource system
    resourceSystemConfig resourceConfig;
    resourceConfig.assetsBasePath = "sandbox/assets";
    resourceConfig.maxLoaderCount = 10;

    resourceSystemInit(&pState->resourceSystemMemoryRequirements, nullptr, resourceConfig);
    pState->resourceSystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->resourceSystemMemoryRequirements);
    if(!resourceSystemInit(&pState->resourceSystemMemoryRequirements, pState->resourceSystem, resourceConfig))
    {
        PFATAL("Resource system could not be initialized! Shutting down now.");
        return false;
    }

    // Init Mesh system
    MeshSystemConfig meshSystemConfig;
    meshSystemConfig.maxMeshesCount = 512;
    meshSystemInit(&pState->meshSystemMemoryRequirements, nullptr, meshSystemConfig);
    pState->meshSystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->meshSystemMemoryRequirements);
    if(!meshSystemInit(&pState->meshSystemMemoryRequirements, pState->meshSystem, meshSystemConfig))
    {
        PFATAL("Mesh system could not be initialized! Shutting down now.");
        return false;
    }

    // Init Texture system.
    TextureSystemConfig textureSystemConfig;
    textureSystemConfig.maxTextureCount = 512;
    textureSystemInit(&pState->textureSystemMemoryRequirements, nullptr, textureSystemConfig);    
    pState->textureSystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->textureSystemMemoryRequirements);
    if(!textureSystemInit(&pState->textureSystemMemoryRequirements, pState->textureSystem, textureSystemConfig))
    {
        PFATAL("Texture system could not be initialized! Shutting down now.");
        return false;
    }

    // Init material system
    MaterialSystemConfig materialSystemConfig;
    materialSystemConfig.maxMaterialCount = 512;
    materialSystemInit(&pState->materialSystemMemoryRequirements, nullptr, materialSystemConfig);
    pState->materialSystem = linearAllocatorAllocate(&pState->systemsAllocator, pState->materialSystemMemoryRequirements);
    if(!materialSystemInit(&pState->materialSystemMemoryRequirements, pState->materialSystem, materialSystemConfig))
    {
        PFATAL("Material system could not be initialized! Shutting down now.");
        return false;
    }

    // Init game
    if(!pState->pGameInst->init(pState->pGameInst))
    {
        PFATAL("Game could not be initialized!");
        return false;
    }

    // Resize game once before begining execution to make sure proper size.
    pState->pGameInst->onResize(pState->pGameInst, pState->m_width, pState->m_height);
    return true;
}

/**
 * Main loop from the application.
 */
bool applicationRun()
{
    u8 frameCount = 0;
    clockStart(&pState->clock);
    clockUpdate(&pState->clock);
    pState->lastTime = pState->clock.elapsedTime;

    while(pState->m_isRunning)
    {
        if(!platformPumpMessages())
            pState->m_isRunning = false;

        if(!pState->m_isSuspended)
        {
            // Update the clock
            clockUpdate(&pState->clock);
            f64 currentTime = pState->clock.elapsedTime; // convert to seconds
            f64 deltaTime = currentTime - pState->lastTime;

            platformUpdate();
            inputSystemUpdate((f32)deltaTime);
            if(!pState->pGameInst->update(pState->pGameInst, (f32)deltaTime))
            {
                PERROR("Game failed to update.");
                pState->m_isRunning = false;
            }
            
            if(!pState->pGameInst->render(pState->pGameInst, (f32)deltaTime))
            {
                PERROR("Game failed to render.");
                pState->m_isRunning = false;
            }

            pState->lastTime = currentTime;
        }
        frameCount++;
    }

    eventUnregister(EVENT_CODE_BUTTON_PRESSED, 0, appOnKey);
    eventUnregister(EVENT_CODE_BUTTON_RELEASED, 0, appOnKey);
    eventUnregister(EVENT_CODE_APP_QUIT, 0, appOnEvent);
    eventUnregister(EVENT_CODE_RESIZED, 0, appOnResize);

    materialSystemShutdown(pState->materialSystem);
    textureSystemShutdown(pState->textureSystem);
    meshSystemShutdown(pState->meshSystem);
    resourceSystemShutdown(pState->resourceSystem);
    renderSystemShutdown(pState->renderSystem);
    inputSystemShutdown(pState->inputSystem);
    platformShutdown(pState->platformSystem);
    eventSystemShutdown(pState->eventSystem);
    memorySystemShutdown(pState->memorySystem);

    return true;
}

bool appOnEvent(u16 code, void* sender, void* listener, eventContext data)
{
    switch (code)
    {
    case EVENT_CODE_APP_QUIT:
        PINFO("EVENT_CODE_APP_QUIT received, shutting down.");
        pState->m_isRunning = false;
        return true;
        break;
    }
    return false;
}

bool appOnResize(u16 code, void* sender, void* listener, eventContext data)
{
    if(code == EVENT_CODE_RESIZED)
    {
        u16 width = data.data.u16[0];
        u16 height = data.data.u16[1];
        PDEBUG("Windows is resized to [%d, %d]!", width, height);

        // If size is different, trigger resize event.
        if(pState->m_width != width || 
            pState->m_height != height)
        {
            pState->m_width = width;
            pState->m_height = height;
            // Minimization
            if(width == 0 || height == 0){
                // If minimized suspend application until it resizes.
                pState->m_isSuspended = true;
                PINFO("Window is minimized.");
                return true;
            }
            else {
                if(pState->m_isSuspended)
                    pState->m_isSuspended = false;
                // Trigger render on resize
                renderOnResize(width, height);
            }
        }
        return true;
    }
    return false;
}

bool appOnKey(u16 code, void* sender, void* listener, eventContext data)
{
    PDEBUG("App on key.");
    if(code == EVENT_CODE_BUTTON_PRESSED)
    {
        u16 keyCode = data.data.u16[0];
        if(keyCode == KEY_ESCAPE)
        {
            // Shutdown application
            eventContext context = {};
            eventFire(EVENT_CODE_APP_QUIT, 0, context);

            // Return to make sure nothing else is processed.
            return true;
        }
        // TODO temp information.
        else if(keyCode == KEY_A)
        {
            PDEBUG("Key A is being pressed!");
        }
        else {
            PDEBUG("A key %c is being pressed!", (keys)keyCode);
        }
        // TODO end temp information.
        return true;
    }
    else if(code == EVENT_CODE_BUTTON_RELEASED)
    {
        u16 keyCode = data.data.u16[0];
        // TODO temp information.
        if(keyCode == KEY_B)
        {
            PDEBUG("Key B has been released!");
        } else {
            PDEBUG("A key has been released!");
        }
        // TODO end temp information.
        return true;
    }

    // let know the event has not been handled.
    return false;
}

void applicationGetFramebufferSize(u32* width, u32* height)
{
    *width = (u32)pState->m_width;
    *height = (u32)pState->m_height;
}