
#include "application.h"
#include "platform/platform.h"
#include "memory/pmemory.h"
#include "event.h"
#include "logger.h"
#include <iostream>

// TODO Remove iostream and use only logger system.

Application::Application()
{
    std::cout << "Application constructor called!" << std::endl;
}

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
        platformSystem, "Pinatsu platform", 100, 100, 400, 400))
    {
        PFATAL("Platform system could not be initialized!");
        return false;
    }
    std::cout << "Platform system initialized ...\n" << std::endl;

    // Init renderer system.


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
    }
    return true;
}