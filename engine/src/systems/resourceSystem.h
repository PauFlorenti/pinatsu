#pragma once

#include "resources/resourcesTypes.h"

typedef struct resourceSystemConfig
{
    u32 maxLoaderCount;
    const char* assetsBasePath;
} resourceSystemConfig;

typedef struct ResourceLoader{
    u32 id;
    resourceTypes type;
    const char* customType;
    const char* typePath;
    bool (*load)(struct ResourceLoader* self, const char* name, Resource* outResource);
    bool (*unload)(struct ResourceLoader* self, Resource* resource);
} ResourceLoader;

bool resourceSystemInit(u64* memoryRequirements, void* state, resourceSystemConfig config);
void resourceSystemShutdown(void* state);

bool resourceSystemRegisterLoader(const ResourceLoader& loader);
bool resourceSystemLoad(const char* name, resourceTypes type, Resource* outResource);
bool resourceSystemCustomLoad(const char* name, const char* customType, Resource* outResource);
void resourceSystemUnload(Resource* resource);

const char* resourceSystemPath(void);

bool load(const char* name, ResourceLoader* loader, Resource* outResource);