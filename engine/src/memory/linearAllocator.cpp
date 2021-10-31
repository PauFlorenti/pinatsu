#include "linearAllocator.h"

#include "pmemory.h"
#include "core/logger.h"

/**
 * This functions create a linear allocator by receiving
 * the total size of the wanted memory, a pointer to that
 * memory and returns the allocator by reference.
 * @param u64 size
 * @param void* memory
 * @param LinearAllocator outAllocator
 */
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
    else
        PWARN("No allocator to be created.");
}

/**
 * Receives the linear allocator to destroy.
 * @param LinearAllocator allocator.
 */
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
    else
        PWARN("Allocator to be destroyed is empty!");
}

/**
 * Given a linear allocator and a size returns a block 
 * of memory of such size allocated by the allocator.
 * @param LinearAllocator allocator
 * @param u64 size
 */
void* linearAllocatorAllocate(LinearAllocator* allocator, u64 size)
{
    if(allocator)
    {
        if((allocator->totalSize - allocator->allocatedSize) < size)
        {
            PERROR("Not enough available space to allocate. Asked for %d but only %d is available.",
                    size, (allocator->totalSize - allocator->allocatedSize));
            return 0;
        }

        void* block = static_cast<u8*>(allocator->memory) + allocator->allocatedSize;
        allocator->allocatedSize += size;
        return block;
    }
    PERROR("No allocator provided. No memory has been allocated.");
    return 0;
}

/**
 * Frees all memory in the allocator.
 * @param LinearAllocator allocator
 */
void linearAllocatorFreeAll(LinearAllocator* allocator)
{
    if(allocator && allocator->memory)
    {
        allocator->allocatedSize = 0;
        memZero(allocator->memory, allocator->totalSize);
    }
}