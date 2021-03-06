#include "platform.h"

#ifndef UNICODE
#define UNICODE
#endif

#include "core\input.h"
#include "core\event.h"
#include "core\logger.h"

#include <iostream>
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>

#include "renderer/vulkan/vulkanTypes.h"
#include <vulkan\vulkan_win32.h>
#include <external/imgui/imgui_impl_win32.h>

LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProcMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct platformState
{
    HWND            hwnd;
    HINSTANCE       hinstance;
    VkSurfaceKHR    surface;
} platformState;

static platformState *pState;

static LARGE_INTEGER initialTime;
static f64 frequencyTimer;

static void setupTimer()
{
    LARGE_INTEGER frequency;    // counts / second
    QueryPerformanceFrequency(&frequency);
    frequencyTimer = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&initialTime);  // counts
}

/**
 * Initialize the platform system by creating
 * a window given a name, position and size.
 */
bool 
platformStartup(
    u64* memoryRequirements,
    void* state,
    const char* name,
    i32 x, i32 y,
    i32 width, i32 height)
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

    u32 clientX = x;
    u32 clientY = y;
    u32 clientWidth = width;
    u32 clientHeight = height;

    u32 windowX = clientX;
    u32 windowY = clientY;
    u32 windowWidth = clientWidth;
    u32 windowHeight = clientHeight;

    DWORD ws_style = 
        WS_MAXIMIZE | WS_MINIMIZE | 
        WS_THICKFRAME | WS_OVERLAPPED | 
        WS_SYSMENU | WS_CAPTION;

    // Get the border size
    RECT border = {0, 0, 0, 0};
    AdjustWindowRectEx(&border, ws_style, 0, WS_EX_APPWINDOW);

    windowX += border.left;
    windowY += border.right;

    windowWidth += border.right - border.left;
    windowHeight += border.bottom - border.top;

    // Create the window.
    HWND handle = CreateWindowEx(
        0, L"Pinatsu window", L"Pinatsu",
        WS_OVERLAPPEDWINDOW,
        //windowX, windowY, windowWidth + 2, windowHeight + 25, nullptr, nullptr, 
        windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, 
        pState->hinstance, nullptr
    );

    if(!handle)
        return false;

    pState->hwnd = handle;

    ShowWindow(pState->hwnd, SW_SHOW);
    return true;
};

void 
platformShutdown(void* state)
{
    if(pState && pState->hwnd){
        DestroyWindow(pState->hwnd);
        pState->hwnd = 0;
    }
}

bool 
platformPumpMessages()
{
    if(pState)
    {
        // Run the message loop.
        MSG msg = {};
        while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return true;
}

void* 
platformAllocateMemory(u64 size)
{
    return malloc(size);
}

void 
platformFreeMemory(void* block)
{
    if(block)
        free(block);
}

void* 
platformZeroMemory(void* block, u64 size)
{
    return memset(block, 0, size);
}

void* 
platformSetMemory(void* dest, i32 value, u64 size)
{
    return memset(dest, value, size);
}

void* 
platformCopyMemory(void* source, void* dest, u64 size)
{
    return memcpy(dest, source, size);
}

void 
platformConsoleWrite(const char* msg, u8 level)
{
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 colour[5] = {64, 4, 6, 1, 2};
    SetConsoleTextAttribute(outputHandle, colour[level]);
    std::cout << msg << std::endl;
    SetConsoleTextAttribute(outputHandle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void 
platformUpdate()
{
    RECT rc;
    GetClientRect(pState->hwnd, &rc);
    RedrawWindow(pState->hwnd, &rc, 0, 0);
}

void
platformSpecificVulkanExtensions(std::vector<const char*>& extensions)
{
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

bool
platformCreateVulkanSurface(VulkanState* vulkanState)
{
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd        = pState->hwnd;
    surfaceInfo.hinstance   = pState->hinstance;

    VkResult result = vkCreateWin32SurfaceKHR(vulkanState->instance, &surfaceInfo, 
                                nullptr, &pState->surface);
    if(result != VK_SUCCESS)
    {
        PFATAL("Win32 Surface could not be created!");
        return false;
    }

    vulkanState->surface = pState->surface;
    return true;
}

LRESULT CALLBACK WinProcMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    if(ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

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
        break;
    }
    case WM_CLOSE: {
        eventContext data = {};
        eventFire(EVENT_CODE_APP_QUIT, 0, data);
        DestroyWindow(hwnd);
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
        break;
    case WM_SYSKEYDOWN: {
        keys key = (keys)wParam;
        inputProcessKey(key, true);
        break;
    }
    case WM_SYSKEYUP: {
        keys key = (keys)wParam;
        inputProcessKey(key, false);
        break;
    }
    case WM_KEYDOWN: {
        keys key = (keys)wParam;
        inputProcessKey(key, true);
        break;
    }
    case WM_KEYUP: {
        keys key = (keys)wParam;
        inputProcessKey(key, false);
        break;
    }
    case WM_MOUSEMOVE: {
        inputProcessMouseMove((i16)GET_X_LPARAM(lParam), (i16)GET_Y_LPARAM(lParam));
        break;
    }
    case WM_LBUTTONDOWN: {
        inputProcessButton(LEFT_MOUSE_BUTTON, true);
        break;
    }
    case WM_LBUTTONUP: {
        inputProcessButton(LEFT_MOUSE_BUTTON, false);
        break;
    }
    case WM_RBUTTONDOWN: {
        inputProcessButton(RIGHT_MOUSE_BUTTON, true);
        break;
    }
    case WM_RBUTTONUP: {
        inputProcessButton(RIGHT_MOUSE_BUTTON, false);
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

f64 platformGetCurrentTime()
{
    // If no timer, start timer.
    if(!frequencyTimer) {
        setupTimer();
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    // return value in micro seconds
    return (f64)now.QuadPart * frequencyTimer;
}

/**
 * @brief Returns the path of the executable.
 * @param void
 * @return const char* Executable path.
 */
const char* getExecutablePath()
{
    CHAR buffer[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos).c_str();
}

void*
platformGetWinHandle()
{
    return pState->hwnd;
}

void setMousePosition(i32 x, i32 y)
{
    SetCursorPos(x, y);
    inputProcessMouseMove(x, y);
}