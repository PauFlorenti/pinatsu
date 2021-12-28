#pragma once

#include "systems/resourceSystem.h"

bool meshLoaderLoad(struct resourceLoader* self, const char* name, Resource* outResource)
{
    if(!self || !name || !outResource)
    {
        return false;
    }

    
}