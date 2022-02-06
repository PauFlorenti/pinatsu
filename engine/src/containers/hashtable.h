#pragma once

#include "defines.h"

struct Hashtable
{
    u64 elementSize;
    u32 elementCount;
    void* memory;
};

void
hashtableCreate(u64 elementSize, u32 elementCount, void* memory, Hashtable* outHastable);

void
hashtableDestroy(Hashtable* hashtable);

bool 
hashtableSetValue(Hashtable* hashtable, const char* name, void* value);

bool
hashtableGetValue(Hashtable* hashtable, const char* name, void* outValue);

bool
hashtableFill(Hashtable* hashtable, void* value);