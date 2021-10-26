#include "linearAllocator.h"

#include "pmemory.h"

void linearAllocatorCreate(u64 size, void* memory, LinearAllocator* outAllocator)
{
    // Check if memory is not null.
    if(outAllocator)
    {
        outAllocator->allocatedSize = 0;
        outAllocator->totalSize     = size;
        outAllocator->ownsMemory    = memory == 0;
        if(memory)
        {
            outAllocator->memory = memory;
        }
        else
        {
            outAllocator->memory = memAllocate(size, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}

void linearAllocatorDestroy(LinearAllocator* allocator)
{
    if(allocator)
    {
        allocator->allocatedSize = 0;
        if(allocator->ownsMemory && allocator->memory)
        {
            memFree(allocator->memory, allocator->totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
        allocator->memory       = nullptr;
        allocator->ownsMemory   = false;
        allocator->totalSize    = 0;
    }
}

void* linearAllocatorAllocate(LinearAllocator* allocator, u64 size)
{
    if(allocator)
    {
        if((allocator->totalSize - allocator->allocatedSize) < size)
        {
            // TODO Logger to report insufficient memory.
            return 0;
        }

        void* block = static_cast<u8*>(allocator->memory) + allocator->allocatedSize;
        allocator->allocatedSize += size;
        return block;
    }
    // TODO Logger to report no allocator is provided.
    return 0;
}

void linearAllocatorFreeAll(LinearAllocator* allocator)
{
    if(allocator && allocator->memory)
    {
        allocator->allocatedSize = 0;
        memZero(allocator->memory, allocator->totalSize);
    }
}