#include "event.h"

#include <vector>
#include "logger.h"
#include "memory/pmemory.h"

struct registeredEvent
{
    void* listener;
    PFN_on_event callback;
};

struct eventsCode
{
    std::vector<registeredEvent> events;
};

// There should not be more than 1000 codes at the moment.
#define MAX_REGISTERED_CODE_EVENTS 1000

struct eventSystemState
{
    eventsCode registered[MAX_REGISTERED_CODE_EVENTS];
    std::vector<registeredEvent>events;
};

static eventSystemState* pState;

/**
 * @brief Initiate the event system. It is called twice. First time receiving a 
 * nullptr as state and returning the necessary memory size.
 * The second time the state is initialized.
 * @param u64* memoryRequirements
 * @param void* state
 * @return void
 */
void eventSystemInit(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(eventSystemState);
    if(!state)
        return;
    
    memZero(state, sizeof(state));
    pState = static_cast<eventSystemState*>(state);
}

/**
 * @brief Shutdown the event system.
 * @param void* state
 * @return void
 */
void eventSystemShutdown(void* state)
{
    pState = 0;
}

/**
 * @brief Register the event with the given code.
 * @param u16 code,
 * @param void* listener
 * @param PFN_on_event on_event function
 * @return bool if succeded.
 */
bool eventRegister(u16 code, void* listener, PFN_on_event on_event)
{
    if(!pState)
        return false;

    if(pState->registered[code].events.empty()){
        //pState->registered[code].events.resize(1);
    }

    for(decltype(pState->registered[code].events.size()) i = 0; i < pState->registered[code].events.size(); ++i){
        if(pState->registered[code].events.at(i).listener == listener){
            // TODO warn ¿?
            return false;
        }
    }

    registeredEvent e{};
    pState->events.resize(10);
    //pState->events.push_back(e);

    pState->registered[code].events.push_back(e);

    registeredEvent event;
    event.callback = on_event;
    event.listener = listener;
    pState->registered[code].events.push_back(event);
    return true;
}

/**
 * @brief Unregister an event given a code.
 * @param u16 code,
 * @param void* listener
 * @param PFN_on_event on_event function
 * @return bool
 */
bool eventUnregister(u16 code, void* listener, PFN_on_event on_event)
{
    if(!pState)
        return false;

    if(pState->registered[code].events.empty()){
        PWARN("Events for code is empty.");
        return false;
    }

    for(decltype(pState->registered[code].events.size()) i = 0;
        i < pState->registered[code].events.size(); ++i)
    {
        if(pState->registered[code].events.at(i).listener == listener)
        {
            auto it = pState->registered[code].events.cbegin();
            pState->registered[code].events.erase(it + i);
            return true;
        }
    }
    // Not found.
    return false;
}

/**
 * @brief Calls the function stored in the code.
 * @param u16 code
 * @param void* sender
 * @param eventContext context
 * @return bool
 */
bool eventFire(u16 code, void* sender, eventContext context)
{
    if(!pState)
        return false;

    if(pState->registered[code].events.empty()){
        PWARN("Events for the given code is empty.");
        return false;
    }

    for(decltype(pState->registered[code].events.size()) i = 0;
        i < pState->registered[code].events.size(); ++i)
    {
        registeredEvent e = pState->registered[code].events.at(i);
        if(e.callback(code, sender, 0, context)){
            // Message handled.
            return true;
        }
    }
    return true;
}