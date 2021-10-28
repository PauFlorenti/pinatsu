#include "vulkanBackend.h"
#include "core/logger.h"
#include "vulkan\vulkan.h"

bool vulkanBackendInit()
{
    PDEBUG("Vulkan backend init entered.");
    return true;
}

void vulkanBackendShutdown()
{

}

bool vulkanBeginFrame()
{
    return true;
}

bool vulkanDraw()
{
    return true;
}

void vulkanEndFrame()
{

}