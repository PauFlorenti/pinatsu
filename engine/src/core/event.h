#pragma once

#include "defines.h"

typedef struct {

    union {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];
    } data;
} eventContext;

typedef bool (*PFN_on_event)(u16 code, void* sender, void* listener, eventContext data);

void eventSystemInit(u64* memoryRequirements, void* state);

void eventSystemShutdown(void* state);

bool eventRegister(u16 code, void* listener, PFN_on_event on_event);

bool eventUnregister(u16 code, void* listener, PFN_on_event on_event);

bool eventFire(u16 code, void* sender, eventContext context);

// System internal event codes
typedef enum SystemEventCode {

    // Shut the application on the next frame.
    EVENT_CODE_APP_QUIT = 0x01,

    EVENT_CODE_KEY_PRESSED = 0x02,

    EVENT_CODE_KEY_RELEASED = 0x03,

    EVENT_CODE_BUTTON_PRESSED = 0x04,

    EVENT_CODE_BUTTON_RELEASED = 0x05,

    EVENT_CODE_MOUSE_MOVED = 0x06,

    EVENT_CODE_MOUSE_WHEEL = 0x07,

    MAX_EVENT_CODE = 0xFF
} SystemEventCode;