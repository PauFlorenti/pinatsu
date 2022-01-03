#pragma once

#include "defines.h"
#include <string>

typedef enum memoryTag
{
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_STACK_ALLOCATOR,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_SCENE,
    MEMORY_TAG_STRING,

    MEMORY_TAG_MAX_TAGS
} memoryTag;

/**
 * Initialize the memory system state given the memory requirements.
 * If state is nullptr, return the memory required, else initialize
 * the system.
 * @param u64* memoryRequirements
 * @param void* state
 * @return void
 */
void memorySystemInit(u64* memoryRequirements, void* state);

/**
 * Shuts down the memory system state.
 * @param void* state
 * @return void
 */
void memorySystemShutdown(void* state);

/**
 * Allocates the size of memory of type tag.
 * @param u64 size
 * @param memoryTag tag
 * @return void* block of memory allocated.
 */
void* memAllocate(u64 size, memoryTag tag);

/**
 * Calls the platform to free a memory block. Size and tag is only
 * for stats purposes.
 * @param void* block
 * @param u64 size
 * @param memoryTag tag
 * @return void
 */
void memFree(void* block, u64 size, memoryTag tag);

/**
 * Zeroes out the block of memory.
 * @param void* block
 * @param u64 size
 * @return void* Zero out block of memory.
 */
void* memZero(void* block, u64 size);

/**
 * Copies the memory data from source to dest.
 * @param void* source
 * @param void* dest
 * @param u64 size
 * @return void* destination block of memory.
 */
void* memCopy(void* source, void* dest, u64 size);

/**
 * Gets the memory usage information of the application
 * and returns it in a string.
 * @param void
 * @return string
 */
std::string getMemoryUsageStr();

