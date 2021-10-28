#include "platform.h"

#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <windows.h>
#include <wingdi.h>
#include "core/event.h"
#include "core/logger.h"

LRESULT CALLBACK WinProcMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct platformState
{
    HWND hwnd;
    HINSTANCE hinstance;
} platformState;

static platformState *pState;

/**
 * Initialize the platform system by creating
 * a window given a name, position and size.
 */
bool platformStartup(
    u64* memoryRequirements,
    void* state,
    const char* name,
    i32 x, i32 y,
    i32 width, i32 height
)
{  
    *memoryRequirements = sizeof(platformState);
    if(!state)
        return true;

    pState = static_cast<platformState*>(state);

    // Register window class.
    pState->hinstance = GetModuleHandle(0);

    WNDCLASS wc = {};

    wc.lpfnWndProc      = WinProcMessage;
    wc.lpszClassName    = L"Pinatsu window";
    wc.hInstance        = pState->hinstance;

    RegisterClass(&wc);

    // Create the window.
    HWND handle = CreateWindowEx(
        0, L"Pinatsu window", L"Pinatsu",
        WS_OVERLAPPEDWINDOW,
        x, y, width, height, nullptr, nullptr, 
        pState->hinstance, nullptr
    );

    if(!handle)
        return false;

    pState->hwnd = handle;

    ShowWindow(pState->hwnd, SW_SHOW);

    std::cout << "platform init called!\n";
    return true;
};

void platformShutdown(void* state)
{
    if(pState && pState->hwnd){
        DestroyWindow(pState->hwnd);
        pState->hwnd = 0;
    }
}

bool platformPumpMessages()
{
    // Run the message loop.
    MSG msg = {};
    while(GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void* platformAllocateMemory(u64 size)
{
    return malloc(size);
}

void platformFreeMemory(void* block)
{
    if(block)
        free(block);
}

void* platformZeroMemory(void* block, u64 size)
{
    return memset(block, 0, size);
}

void* platformCopyMemory(void* source, void* dest, u64 size)
{
    return memcpy(dest, source, size);
}

void platformConsoleWrite(const char* msg, u8 level)
{
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 colour[5] = {64, 4, 6, 1, 2};
    SetConsoleTextAttribute(outputHandle, colour[level]);
    std::cout << msg << std::endl;
    SetConsoleTextAttribute(outputHandle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void platformUpdate()
{
    RECT rc;
    GetClientRect(pState->hwnd, &rc);
    RedrawWindow(pState->hwnd, &rc, 0, 0);
}

LRESULT CALLBACK WinProcMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE: {
        // Get new size
        RECT clientRect;
        if(!GetClientRect(hwnd, &clientRect))
        {
            PERROR("Error getting new client size!");
            return 0;
        }

        eventContext context = {};
        context.data.u16[0] = (u16)(clientRect.right - clientRect.left);
        context.data.u16[1] = (u16)(clientRect.bottom - clientRect.top);
        eventFire(EVENT_CODE_RESIZED, 0, context);
        return 0;
        break;
    }
    case WM_CLOSE: {
        eventContext data = {};
        PINFO("Close window event received, proceeding to quit application.");
        eventFire(EVENT_CODE_APP_QUIT, 0, data);
        DestroyWindow(hwnd);
        return 0;
        break;
    }
    case WM_DESTROY:
        // TODO Handled unexpected window destruction.
        //PWARN("Window destroyed unexpectedly!");
        PostQuitMessage(0);
        return 0;
        break;
    case WM_PAINT:
        //PDEBUG("WM_PAINT is being called,.");
        return 0;
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}