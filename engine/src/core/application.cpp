
#include "application.h"
#include "platform/platform.h"
#include "memory/pmemory.h"
#include "event.h"
#include "input.h"
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
bool appOnKey(u16 code, void* sender, void* listener, eventContext data);

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
    inputSystemInit(&inputSystemMemoryRequirements, nullptr);
    inputSystem = linearAllocatorAllocate(&systemsAllocator, inputSystemMemoryRequirements);
    inputSystemInit(&inputSystemMemoryRequirements, inputSystem);
    eventRegister(EVENT_CODE_BUTTON_PRESSED, 0, appOnKey);
    eventRegister(EVENT_CODE_BUTTON_RELEASED, 0, appOnKey);

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
            f64 currentTime = clock.elapsedTime * 1000000;
            f64 deltaTime = currentTime - lastTime;
            //printf("CurrentTime: %Lf - DeltaTime: %Lf\n", currentTime, deltaTime);

            platformUpdate();
            inputSystemUpdate((f32)deltaTime);

            // Draw
            // TODO Create a struct Game with its own function pointers to
            // Update and render
            // At the moment update the scene is done in renderBeginFrame and it shouldn't
            if(renderBeginFrame((f32)deltaTime)){
                renderDrawFrame();
                renderEndFrame(1.0f);
            }

            lastTime = currentTime;
        }
        frameCount++;
    }

    eventUnregister(EVENT_CODE_BUTTON_PRESSED, 0, appOnKey);
    eventUnregister(EVENT_CODE_BUTTON_RELEASED, 0, appOnKey);
    eventUnregister(EVENT_CODE_APP_QUIT, 0, appOnEvent);
    eventUnregister(EVENT_CODE_RESIZED, 0, appOnResize);

    renderSystemShutdown(renderSystem);
    inputSystemShutdown(inputSystem);
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