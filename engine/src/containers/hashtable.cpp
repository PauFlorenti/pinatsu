#include "hashtable.h"

#include "core/logger.h"
#include "memory/pmemory.h"

static u64
makeHash(const char* name, u32 elementCount)
{
    static const u64 multiplier = 97;
    u64 hash = 0;

    u32 length = strlen(name);

    for(u32 i = 0; i < length; ++i) {
        hash = hash * multiplier + name[i];
    }

    hash %= elementCount;
    return hash;
}

void
hashtableCreate(u64 elementSize, u32 elementCount, void* memory, Hashtable* outHashtable)
{
    if(!memory || !outHashtable) {
        PERROR("hashtable creation failed! Memory or outHashtable pointers are not provided.");
        return;
    }

    if(!elementCount || !elementSize) {
        PERROR("hastable creation failed! elementCount or elementSize must be non-zero values.");
        return;
    }

    outHashtable->memory        = memory;
    outHashtable->elementCount  = elementCount;
    outHashtable->elementSize   = elementSize;
    memZero(outHashtable->memory, elementSize * elementCount);
}

void
hashtableDestroy(Hashtable* hashtable) 
{
    if(hashtable) {
        memZero(hashtable, sizeof(hashtable));
    }
}

bool 
hashtableSetValue(Hashtable* hashtable, const char* name, void* value)
{
    if(!hashtable || !name || !value) {
        PERROR("hashtableSetValue error! A valid hashtable, name and value must be provided.");
        return false;
    }

    u64 hash = makeHash(name, hashtable->elementCount);
    memCopy(value, ((char*)hashtable->memory) + (hashtable->elementSize * hash), hashtable->elementSize);
    return true;
}

bool
hashtableGetValue(Hashtable* hashtable, const char* name, void** outValue)
{
    if(!hashtable || !name) {
        PERROR("hashtableGetValue - a valid hashtable or name must be provided!");
        return false;
    }

    u64 hash = makeHash(name, hashtable->elementCount);
    *outValue = ((char*)hashtable->memory) + (hashtable->elementSize * hash);
    return true;
}

bool
hashtableGetByPos(Hashtable* hashtable, u32 position, void* outValue)
{
    if(!hashtable) {
        return false;
    }

    outValue = ((char*)hashtable->memory + (hashtable->elementSize * position));
    return true;
}

bool
hashtableFill(Hashtable* hashtable, void* value)
{
    if(!hashtable || !value) {
        PERROR("Could not fill hashtable! Hashtable or value must be valid.");
        return false;
    }

    for(u32 i = 0; i < hashtable->elementCount; i++) {
        memCopy(value, (u8*)hashtable->memory + (hashtable->elementSize * i), hashtable->elementSize);
    }
    return true;
}


// TODO make a way to dehash the value.
void
hashtablePrint(Hashtable* hashtable)
{
    if(!hashtable) {
        PERROR("hashtablePrint - table passed is not valid.");
        return;
    }

    for(u32 i = 0; i < hashtable->elementCount; ++i) {
        //hashtable->
    }
}