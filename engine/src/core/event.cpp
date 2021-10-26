#include "event.h"

#include <vector>

typedef struct registeredEvent
{
    void* listener;
    PFN_on_event callback;
} registeredEvent;

typedef struct eventsCode
{
    std::vector<registeredEvent> events;
} eventsCode;

// There should not be more than 1000 codes at the moment.
#define MAX_REGISTERED_CODE_EVENTS 1000

typedef struct eventSystemState
{
    eventsCode registered[MAX_REGISTERED_CODE_EVENTS];
} eventSystemState;

static eventSystemState* pState;

void eventSystemInit(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(eventSystemState);
    if(!state)
        return;
    
    pState = static_cast<eventSystemState*>(state);
}

void eventSystemShutdown(void* state)
{
}

bool eventRegister(u16 code, void* listener, PFN_on_event on_event)
{
    return true;
}

bool eventUnregister(u16 code, void* listener, PFN_on_event on_event)
{
    return true;
}

bool eventFire(u16 code, void* sender, eventContext context)
{
    return true;
}