#pragma once

#include "defines.h"

#define LOG_WARN_ENABLED true
#define LOG_DEBUG_ENABLED true
#define LOG_INFO_ENABLED true

// TODO Should we disable debug and info logs
// in a release version? - Debug for sure.

typedef enum LogLevel
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_INFO = 4
} LogLevel;

bool loggerInit(u64* memoryRequirements, void* state);
void loggerShutdown(void* state);

void logOut(LogLevel level, const char* msg, ...);

#define PFATAL(msg, ...) logOut(LOG_LEVEL_FATAL, msg, ##__VA_ARGS__);

#ifndef PERROR
#define PERROR(msg, ...) logOut(LOG_LEVEL_ERROR, msg, ##__VA_ARGS__);
#endif

#ifndef PWARN
#define PWARN(msg, ...) logOut(LOG_LEVEL_WARN, msg, ##__VA_ARGS__);
#endif

#ifndef PDEBUG
#define PDEBUG(msg, ...) logOut(LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__);
#endif

#ifndef PINFO
#define PINFO(msg, ...) logOut(LOG_LEVEL_INFO, msg, ##__VA_ARGS__);
#endif