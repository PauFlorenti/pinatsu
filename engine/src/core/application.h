#pragma once

#include "defines.h"
#include "memory/linearAllocator.h"

class Application
{
public:
    Application();

    static Application* m_instance;

    static Application* getInstance()
    {
        if(!m_instance)
            m_instance = new Application();
            
        return m_instance;
    }

    bool init();
    bool run();

    bool m_isRunning;
    i16 m_width;
    i16 m_height;
    LinearAllocator systemsAllocator;

    u64 memorySystemMemoryRequirements;
    void* memorySystem;

    u64 logSystemMemoryRequirements;
    void* logSystem;

    u64 eventSystemMemoryRequirements;
    void* eventSystem;

    u64 inputSystemMemoryRequirements;
    void* inputSystem;

    u64 platformSystemMemoryRequirements;
    void* platformSystem;

    u64 renderSystemMemoryRequirements;
    void* renderSystem;

private:
};

Application* CreateApplication();