#include "input.h"

#include "memory/pmemory.h"
#include "logger.h"
#include "event.h"

typedef struct KeyboardState
{
    bool keys[256];
} KeyboardState;

typedef struct MouseState
{
    i16 x;
    i16 y;
    u8 buttons[BUTTONS_MAX_BUTTONS];
    
} MouseState;

typedef struct InputState
{
    KeyboardState currentKeyboard;
    KeyboardState previousKeyboard;
    MouseState currentMouse;
    MouseState previousMouse;
} InputState;

static InputState* pState;

void inputSystemInit(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(InputState);
    if(!state)
        return;
    
    memZero(state, sizeof(state));
    pState = static_cast<InputState*>(state);
}

void inputSystemShutdown(void* state)
{
    pState = nullptr;
}

void inputSystemUpdate(f32 deltaTime)
{
    if(!pState)
        return;

    // Copy the current state to the previous state.
    memCopy(&pState->currentKeyboard, &pState->previousKeyboard, sizeof(KeyboardState));
    memCopy(&pState->currentMouse, &pState->previousMouse, sizeof(MouseState));
}

// Keyboard Input
bool isKeyDown(keys key)
{
    if(!pState)
        return false;
    return pState->currentKeyboard.keys[key] == true;
}

bool isKeyUp(keys key)
{
    if(!pState)
        return true;
    return pState->currentKeyboard.keys[key] == false;
}

bool wasKeyUp(keys key)
{
    if(!pState)
        return true;
    return pState->previousKeyboard.keys[key] == false;
}

bool wasKeyDown(keys key)
{
    if(!pState)
        return false;
    return pState->previousKeyboard.keys[key] == true;
}

// Mouse input
bool isMouseButtonUp(Buttons button)
{
    if(!pState)
        return true;
    return pState->currentMouse.buttons[button] == false;
}

bool isMouseButtonDown(Buttons button)
{
    if(!pState)
        return false;
    return pState->currentMouse.buttons[button] == true;
}

bool wasMouseButtonUp(Buttons button)
{
    if(!pState)
        return true;
    return pState->previousMouse.buttons[button] == false;
}

bool wasMouseButtonDown(Buttons button)
{
    if(!pState)
        return false;
    return pState->previousMouse.buttons[button] == true;
}

void getMousePosition(i32* x, i32* y)
{
    if(!pState)
    {
        *x = 0;
        *y = 0;
        return;
    }
    *x = pState->currentMouse.x;
    *y = pState->currentMouse.y;
}

void getPreviousMousePosition(i32* x, i32* y)
{
    if(!pState)
    {
        *x = 0;
        *y = 0;
        return;
    }
    *x = pState->previousMouse.x;
    *y = pState->previousMouse.y;
}

void inputProcessKey(keys key, bool pressed)
{
    // Handle it if it has not been handled or state has changed
    if(pState != nullptr && pState->currentKeyboard.keys[key] != pressed)
    {
        pState->currentKeyboard.keys[key] = pressed;

        if(key == KEY_LALT)
        {
            PWARN("Left Alt %s.", pressed ? "Pressed" : "Released");
        } else if(key == KEY_RALT) {
            PWARN("Right Alt %s.", pressed ? "Pressed" : "Released");
        }
        if(key == KEY_LCONTROL)
        {
            PWARN("Left Control %s.", pressed ? "Pressed" : "Released");
        } else if (key == KEY_RCONTROL) {
            PWARN("Right Control %s.", pressed ? "Pressed" : "Released");
        }
        if(key == KEY_LSHIFT)
        {
            PWARN("Left Shift %s.", pressed ? "Pressed" : "Released");
        } else if( key == KEY_RSHIFT) {
            PWARN("Right Shift %s.", pressed ? "Pressed" : "Released");
        }

        // Fire event
        eventContext context;
        context.data.u16[0] = key;
        eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void inputProcessButton(Buttons button, bool pressed)
{
    if(pState != nullptr && pState->currentMouse.buttons[button] != pressed)
    {
        pState->currentMouse.buttons[button] = pressed;

        // Fire event
        eventContext context;
        context.data.u16[0] = button;
        eventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}