#pragma once

#include "resources/resourcesTypes.h"

typedef struct resourceSystemConfig
{
    u32 maxLoaderCount;
    const char* assetsBasePath;
} resourceSystemConfig;

typedef struct resourceLoader{
    u32 id;
    resourceTypes type;
    const char* customType;
    const char* typePath;
    bool (*load)(struct resourceLoader* self, const char* name, Resource* outResource);
    bool (*unload)(struct resourceLoader* self, Resource* resource);
} resourceLoader;

bool resourceSystemInit(u64* memoryRequirements, void* state, resourceSystemConfig config);
void resourceSystemShutdown(void* state);

bool resourceSystemRegisterLoader(resourceLoader loader);
bool resourceSystemLoad(const char* name, resourceTypes type, Resource* outResource);
bool resourceSystemCustomLoad(const char* name, const char* customType, Resource* outResource);
void resourceSystemUnload(Resource* resource);

bool load(const char* name, resourceLoader* loader, Resource* outResource);