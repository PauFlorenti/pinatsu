
#include "application.h"
#include "platform/platform.h"
#include "memory/pmemory.h"
#include "event.h"
#include <iostream>

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

    // Init event system.
    std::cout << "Initializing event system ..." << std::endl;
    eventSystemInit(&eventSystemMemoryRequirements, nullptr);
    eventSystem = linearAllocatorAllocate(&systemsAllocator, eventSystemMemoryRequirements);
    eventSystemInit(&eventSystemMemoryRequirements, eventSystem);
    std::cout << "Event system initialized ..." << std::endl;

    // Init platform system.
    std::cout << "Initializing platform system ..." << std::endl;
    platformStartup(&platformSystemMemoryRequirements, 0, 0, 0, 0, 0, 0);
    platformSystem = linearAllocatorAllocate(&systemsAllocator, platformSystemMemoryRequirements);
    if(!platformStartup(&platformSystemMemoryRequirements, 
        platformSystem, "Pinatsu platform", 100, 100, 400, 400))
    {
        std::cout << "Platform system could not be initialized" << std::endl;
        return false;
    }
    std::cout << "Platform system initialized ...\n" << std::endl;

    platformConsoleWrite("This is a message written from platform!", 7);

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