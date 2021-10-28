#include "logger.h"

#include "platform\platform.h"
#include <string>
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
    size_t length = message.size();
    char * outBuffer = new char[length + 1];

    va_list args;
    va_start(args, msg);
    std::vsnprintf(outBuffer, length, message.c_str(), args);
    va_end(args);

    platformConsoleWrite(outBuffer, level);
}