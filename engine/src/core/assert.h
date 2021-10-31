#pragma once

#include "defines.h"

void reportAssertionFailure(const char* expression, const char* message, const char* file, i32 line);

#define ASSERTIONS_ENABLED

#ifdef ASSERTIONS_ENABLED
#ifdef _MSC_VER
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif
#endif

// TODO file and line not returning values.

#define PASSERT(asrt) {                                             \
    if(asrt){                                                       \
    }                                                               \
    else{                                                           \
        reportAssertionFailure(#asrt, "", __FILE__, __LINE__);      \
        debugBreak();                                               \
    }                                                               \
}                                                                   \

#define PASSERT_MSG(asrt, msg) {                                    \
    if(asrt){                                                       \
    }                                                               \
    else{                                                           \
        reportAssertionFailure(#asrt, msg, __FILE__, __LINE__);     \
        debugBreak();                                               \
    }                                                               \
}                                                                   \