
#include "application.h"
#include "platform/platform.h"
#include "memory/pmemory.h"
#include "event.h"
#include "logger.h"
#include "renderer/rendererFrontend.h"
#include <iostream>

// TODO Remove iostream and use only logger system.

Application* Application::m_instance = nullptr;

Application::Application()
{
    m_instance = this;
    std::cout << "Application constructor called!" << std::endl;
}

bool appOnEvent(u16 code, void* sender, void* listener, eventContext data);
bool appOnResize(u16 code, void* sender, void* listener, eventContext data);

/**
 * Initialize all systems neede for the app to run.
 */
bool Application::init()
{
    m_isRunning = true;

    PFATAL("This is a fatal test!");
    PERROR("This is an error test!");
    PWARN("This is a warning test!");
    PDEBUG("This is a debug test!");
    PINFO("This is a info test!");

    u64 systemsAllocatorTotalSize = 64 * 1024 * 1024; // 64mb
    std::cout << "Initializing linear allocator with " << systemsAllocatorTotalSize << " Bytes.\n";
    linearAllocatorCreate(systemsAllocatorTotalSize, 0, &systemsAllocator);

    // TODO Create logger to log back all info properly.

    //! Init Subsystems
    // Init memory system
    std::cout << "Initializing memory system ..." << std::endl;
    memorySystemInit(&memorySystemMemoryRequirements, nullptr);
    memorySystem = linearAllocatorAllocate(&systemsAllocator, memorySystemMemoryRequirements);
    memorySystemInit(&memorySystemMemoryRequirements, memorySystem);
    std::cout << "Memory system initialized ..." << std::endl;

    // Init logger system if needed.

    // Init event system.
    std::cout << "Initializing event system ..." << std::endl;
    eventSystemInit(&eventSystemMemoryRequirements, nullptr);
    eventSystem = linearAllocatorAllocate(&systemsAllocator, eventSystemMemoryRequirements);
    eventSystemInit(&eventSystemMemoryRequirements, eventSystem);
    std::cout << "Event system initialized ..." << std::endl;

    // Init input system.

    // Init platform system.
    std::cout << "Initializing platform system ..." << std::endl;
    platformStartup(&platformSystemMemoryRequirements, 0, 0, 0, 0, 0, 0);
    platformSystem = linearAllocatorAllocate(&systemsAllocator, platformSystemMemoryRequirements);
    if(!platformStartup(&platformSystemMemoryRequirements, 
        platformSystem, "Pinatsu platform", 100, 100, 1200, 800))
    {
        PFATAL("Platform system could not be initialized!");
        return false;
    }
    std::cout << "Platform system initialized ...\n" << std::endl;
    
    eventRegister(EVENT_CODE_APP_QUIT, 0, appOnEvent);
    eventRegister(EVENT_CODE_RESIZED, 0, appOnResize);

    // Init renderer system.
    renderSystemInit(&renderSystemMemoryRequirements, nullptr, nullptr);
    renderSystem = linearAllocatorAllocate(&systemsAllocator, renderSystemMemoryRequirements);
    renderSystemInit(&renderSystemMemoryRequirements, renderSystem, "Pinatsu engine");

    std::cout << getMemoryUsageStr() << std::endl;

    return true;
}

/**
 * Main loop from the application.
 */
bool Application::run()
{
    while(m_isRunning)
    {
        if(!platformPumpMessages())
            m_isRunning = false;

        platformUpdate();
    }

    eventUnregister(EVENT_CODE_APP_QUIT, 0, appOnEvent);
    eventUnregister(EVENT_CODE_RESIZED, 0, appOnResize);

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
            // Minimization
            if(width == 0 || height == 0){
                PINFO("Window is minimized.");
                return true;
            }
            else {
                // Trigger applicaiton on resize
                // Trigger render on resize
            }
        }
        return true;
    }
    return false;
}