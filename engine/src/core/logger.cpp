#include "logger.h"

#include "platform\platform.h"
#include "assert.h"
#include <cstdarg>
#include <memory>

bool loggerInit(u64* memoryRequirements, void* state)
{
    // TODO Should be implemented if logs are written to a file.log
    return true;
}

void loggerShutdown(void* state)
{
    //! TODO Should be implemented if logs are written to a file.log
}

void logOut(LogLevel level, const char* msg, ...)
{
    bool isError = level < LOG_LEVEL_WARN;

    const char* levelStrings[5] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[DEBUG]: ", "[INFO]: "};

    std::string message = std::string(levelStrings[level]) + std::string(msg);
    size_t length = message.size() + 1;
    // Find a way to make it dynamic.
    char * outBuffer = new char[1000];

    va_list args;
    va_start(args, msg);
    std::vsnprintf(outBuffer, 1000, message.c_str(), args);
    va_end(args);

    platformConsoleWrite(outBuffer, level);
}

void reportAssertionFailure(const char* expression, const char* msg, const char* file, i32 line)
{
    logOut(LOG_LEVEL_FATAL, "Assert failed: %s, message %s, file %s, line %d\n", expression, msg, file, line);
}