#include "logger.h"

#include "platform\platform.h"
#include <string>

bool loggerInit(u64* memoryRequirements, void* state)
{
    // TODO Should be implemented if logs are written to a file.log
    return true;
}

void loggerShutdown(void* state)
{
    //! TODO Should be implemented if logs are written to a file.log
}

void logOut(LogLevel level, const char* msg)
{
    bool isError = level < LOG_LEVEL_WARN;

    const char* levelStrings[5] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[DEBUG]: ", "[INFO]: "};

    std::string message = std::string(levelStrings[level]) + std::string(msg);

    platformConsoleWrite(message.c_str(), level);
}