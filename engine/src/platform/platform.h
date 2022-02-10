#pragma once

#include "../defines.h"
#include "renderer/vulkan/vulkanTypes.h"
#include <vector>

bool platformStartup(
    u64* memoryRequirements,
    void* state,
    const char* name,
    i32 x, i32 y,
    i32 width, i32 height
);

void platformShutdown(void* state);

bool platformPumpMessages();

void* platformAllocateMemory(u64 size);

void platformFreeMemory(void* block);

void* platformZeroMemory(void* block, u64 size);

void* platformSetMemory(void* dest, i32 value, u64 size);

void* platformCopyMemory(void* source, void* dest, u64 size);

void platformConsoleWrite(const char* msg, u8 level);

void platformUpdate();

void platformSpecificExtensions(std::vector<const char*>& extensions);

bool platformSurfaceCreation(VulkanState* vulkanState);

f64 platformGetCurrentTime();

const char* getExecutablePath();

void* platformGetWinHandle();