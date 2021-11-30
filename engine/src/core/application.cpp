
#include "application.h"
#include "platform/platform.h"
#include "memory/pmemory.h"
#include "event.h"
#include "logger.h"
#include "assert.h"
#include "renderer/rendererFrontend.h"

#include <iostream>

Application* Application::m_instance = nullptr;

Application::Application()
{
    m_instance = this;
}

bool appOnEvent(u16 code, void* sender, void* listener, eventContext data);
bool appOnResize(u16 code, void* sender, void* listener, eventContext data);

/**
 * Initialize all systems neede for the app to run.
 */
bool Application::init()
{
    m_isRunning = true;
    m_isSuspended = false;

    PFATAL("This is a fatal test!");
    PERROR("This is an error test!");
    PWARN("This is a warning test!");
    PDEBUG("This is a debug test!");
    PINFO("This is a info test!");

    // TODO Game info should get pass for properties such as width and height.
    m_width = 1200;
    m_height = 800;

    u64 systemsAllocatorTotalSize = 64 * 1024 * 1024; // 64mb
    linearAllocatorCreate(systemsAllocatorTotalSize, 0, &systemsAllocator);

    //! Init Subsystems
    // Init memory system
    memorySystemInit(&memorySystemMemoryRequirements, nullptr);
    memorySystem = linearAllocatorAllocate(&systemsAllocator, memorySystemMemoryRequirements);
    memorySystemInit(&memorySystemMemoryRequirements, memorySystem);

    // Init logger system if needed.

    // Init event system.
    eventSystemInit(&eventSystemMemoryRequirements, nullptr);
    eventSystem = linearAllocatorAllocate(&systemsAllocator, eventSystemMemoryRequirements);
    eventSystemInit(&eventSystemMemoryRequirements, eventSystem);

    // Init input system.

    // Init platform system.
    platformStartup(&platformSystemMemoryRequirements, 0, 0, 0, 0, 0, 0);
    platformSystem = linearAllocatorAllocate(&systemsAllocator, platformSystemMemoryRequirements);
    if(!platformStartup(&platformSystemMemoryRequirements, 
        platformSystem, "Pinatsu platform", 100, 100, m_width, m_height))
    {
        PFATAL("Platform system could not be initialized!");
        return false;
    }

    eventRegister(EVENT_CODE_APP_QUIT, 0, appOnEvent);
    eventRegister(EVENT_CODE_RESIZED, 0, appOnResize);

    // Init renderer system.
    renderSystemInit(&renderSystemMemoryRequirements, nullptr, nullptr);
    renderSystem = linearAllocatorAllocate(&systemsAllocator, renderSystemMemoryRequirements);
    if(!renderSystemInit(&renderSystemMemoryRequirements, renderSystem, "Pinatsu engine"))
    {
        PFATAL("Render system could not be initialized! Shuting down now.");
        return false;
    }
    PINFO(getMemoryUsageStr().c_str());
    return true;
}

/**
 * Main loop from the application.
 */
bool Application::run()
{
    u8 frameCount = 0;
    // TODO Clock timer to compute delta time.
    clockStart(&clock);
    clockUpdate(&clock);
    lastTime = clock.elapsedTime;

    while(m_isRunning)
    {
        if(!platformPumpMessages())
            m_isRunning = false;

        if(!m_isSuspended)
        {
            // Update the clock
            clockUpdate(&clock);
            f64 currentTime = clock.elapsedTime;
            f64 deltaTime = currentTime - lastTime;

            platformUpdate();

            // Draw
            // TODO Create a struct Game with its own function pointers to
            // Update and render
            // At the moment update the scene is done in renderBeginFrame and it shouldn't
            if(renderBeginFrame(deltaTime)){
                renderDrawFrame();
                renderEndFrame(1.0f);
            }

            lastTime = currentTime;
        }
        frameCount++;
    }

    eventUnregister(EVENT_CODE_APP_QUIT, 0, appOnEvent);
    eventUnregister(EVENT_CODE_RESIZED, 0, appOnResize);

    renderSystemShutdown(renderSystem);
    platformShutdown(platformSystem);
    eventSystemShutdown(eventSystem);
    memorySystemShutdown(memorySystem);

    return true;
}

bool appOnEvent(u16 code, void* sender, void* listener, eventContext data)
{
    switch (code)
    {
    case EVENT_CODE_APP_QUIT:
        PINFO("EVENT_CODE_APP_QUIT received, shutting down.");
        Application::getInstance()->m_isRunning = false;
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
        if(Application::getInstance()->m_width != width || 
            Application::getInstance()->m_height != height)
        {
            Application::getInstance()->m_width = width;
            Application::getInstance()->m_height = height;
            // Minimization
            if(width == 0 || height == 0){
                // If minimized suspend application until it resizes.
                Application::getInstance()->m_isSuspended = true;
                PINFO("Window is minimized.");
                return true;
            }
            else {
                if(Application::getInstance()->m_isSuspended)
                    Application::getInstance()->m_isSuspended = false;
                // Trigger render on resize
                renderOnResize(width, height);
            }
        }
        return true;
    }
    return false;
}