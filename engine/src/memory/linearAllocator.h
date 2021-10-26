#pragma once

#include "defines.h"

/**
 * Linear allocator functions.
 * A linear or arena allocator only saves a pointer
 * to its last occupied memory address. It can ask for
 * more memory but it can only free them all at once.  
 */ 

typedef struct LinearAllocator
{
    u64 totalSize;
    u64 allocatedSize;
    void* memory;
    bool ownsMemory;
} LinearAllocator;

/**
 * This functions create a linear allocator by receiving
 * the total size of the wanted memory, a pointer to that
 * memory and returns the allocator by reference.
 * @param u64 size
 * @param void* memory
 * @param LinearAllocator outAllocator
 */
void linearAllocatorCreate(u64 size, void* memory, LinearAllocator* outAllocator);

/**
 * Receives the linear allocator to destroy.
 * @param LinearAllocator allocator.
 */
void linearAllocatorDestroy(LinearAllocator* allocator);

/**
 * Given a linear allocator and a size returns a block 
 * of memory of such size allocated by the allocator.
 * @param LinearAllocator allocator
 * @param u64 size
 */
void* linearAllocatorAllocate(LinearAllocator* allocator, u64 size);

/**
 * Frees all memory in the allocator.
 * @param LinearAllocator allocator
 */
void linearAllocatorFreeAll(LinearAllocator* allocator);