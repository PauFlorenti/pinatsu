#include "platform.h"

#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <windows.h>

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

LRESULT CALLBACK WinProcMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}