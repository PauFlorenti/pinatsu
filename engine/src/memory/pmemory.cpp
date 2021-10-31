#include "pmemory.h"

#include "platform/platform.h"
#include "core\logger.h"

struct memoryStats
{
    u64 totalAllocated;
    u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memoryTagsStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "LINEAR_ALLOC",
    "STACK_ALLOC",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "SCENE      "
};

typedef struct memorySystemState
{
    struct memoryStats stats;
    u64 allocCount;
} memorySystemState;

static memorySystemState* pState;

void memorySystemInit(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(memorySystemState);
    if(!state)
        return;

    pState = static_cast<memorySystemState*>(state);
    pState->allocCount = 0;

    platformZeroMemory(&pState->stats, sizeof(memoryStats));
}

void memorySystemShutdown(void* state)
{
    pState = nullptr;
}

void* memAllocate(u64 size, memoryTag tag)
{
    if(tag == MEMORY_TAG_UNKNOWN){
        PWARN("Memory tag is UNKNOWN.");
    }

    if(pState)
    {
        pState->stats.totalAllocated += size;
        pState->stats.taggedAllocations[tag] += size;
        pState->allocCount++;
    }

    void* block = platformAllocateMemory(size);
    return platformZeroMemory(block, sizeof(block));
}

void memFree(void* block, u64 size, memoryTag tag)
{
    if(tag == MEMORY_TAG_UNKNOWN)
    {
        PWARN("Memory tag is UNKNOWN.");
    }

    if(pState)
    {
        pState->stats.totalAllocated -= size;
        pState->stats.taggedAllocations[tag] -= size;
        pState->allocCount--;
    }
    platformFreeMemory(block);
}

void* memZero(void* block, u64 size)
{
    return platformZeroMemory(block, size);
}

void* memCopy(void* source, void* dest, u64 size)
{
    return platformCopyMemory(source, dest, size);
}

std::string getMemoryUsageStr()
{
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;
    std::string title = "System memory use (tagged):\n";
    std::string str;
    for( u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i )
    {
        float amount = 1.0f;
        std::string unit = "XiB";
        if(pState->stats.taggedAllocations[i] > gib)
        {
            unit[0] = 'G';
            amount = pState->stats.taggedAllocations[i] / (float)gib;
        }
        else if(pState->stats.taggedAllocations[i] > mib)
        {
            unit[0] = 'M';
            amount = pState->stats.taggedAllocations[i] / (float)mib;

        }
        else if(pState->stats.taggedAllocations[i] > kib)
        {
            unit[0] = 'K';
            amount = pState->stats.taggedAllocations[i] / (float)kib;
        }
        else
        {
            unit[0] = 'B';
            unit[1] = unit[2] = '\0';
            amount = (float)pState->stats.taggedAllocations[i];
        }
        std::string aux = memoryTagsStrings[i];
        str += "\t" + aux + " " + std::to_string(amount).substr(0, std::to_string(amount).find('.') + 3) + " " + unit + "\n";
    }
    return title + str;
} 