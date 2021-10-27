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

void linearAllocatorCreate(u64 size, void* memory, LinearAllocator* outAllocator);

void linearAllocatorDestroy(LinearAllocator* allocator);

void* linearAllocatorAllocate(LinearAllocator* allocator, u64 size);

void linearAllocatorFreeAll(LinearAllocator* allocator);