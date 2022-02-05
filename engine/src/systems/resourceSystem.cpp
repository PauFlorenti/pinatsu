#include "resourceSystem.h"

#include "core/logger.h"
#include "resources/loaders/meshLoader.h"
#include "resources/loaders/textureLoader.h"
#include "resources/loaders/gltfLoader.h"

// TODO Make own string container funcs.
#include <cstring>

typedef struct ResoureSystemState{
    resourceSystemConfig config;
    ResourceLoader* loaders;
} ResourceSystemState;

static ResourceSystemState* pState = nullptr;

bool resourceSystemInit(u64* memoryRequirements, void* state, resourceSystemConfig config)
{
    // Check the config is valid.
    if(config.maxLoaderCount < 1){
        PERROR("Maximum loaders allowed is below 1. That is an error, shutting down.");
        return false;
    }
    if(!config.assetsBasePath) {
        PERROR("resourceSystemInit - No assets base path has been given. Shutting down.");
        return false;
    }

    // If system not init yet, return memory requirements to be initialized.
    if(state == nullptr)
    {
        *memoryRequirements = sizeof(ResourceSystemState) + (sizeof(ResourceLoader) * (config.maxLoaderCount + 1));
        return true;
    }

    pState = static_cast<ResourceSystemState*>(state);
    pState->config = config;

    void* resourceLoaderPtr = pState + sizeof(ResourceSystemState);
    pState->loaders = (ResourceLoader*)(resourceLoaderPtr);

    for(u32 i = 0; i < pState->config.maxLoaderCount; i++) {
        pState->loaders[i].id = INVALID_ID;
    }

    // Register default known loaders.
    resourceSystemRegisterLoader(meshLoaderCreate());
    resourceSystemRegisterLoader(textureLoaderCreate());
    resourceSystemRegisterLoader(gltfLoaderCreate());

    PINFO("Resource system initialized with base path %s.", config.assetsBasePath);

    return true;
}

void resourceSystemShutdown(void* state)
{
    if(state)
        state = nullptr;
}

bool resourceSystemRegisterLoader(const ResourceLoader& loader)
{
    if(pState)
    {
        u32 count = pState->config.maxLoaderCount;
        // Make sure resource loader is not already registered
        for(u32 i = 0; i < count; i++)
        {
            ResourceLoader* l = &pState->loaders[i];
            if(l->id != INVALID_ID)
            {
                if(l->type == loader.type)
                {
                    PERROR("resoureSystemRegisterLoader - Loader of type %s is already registered and will not be registered again.", loader.type);
                    return false;
                }
                // TODO check if custom loader already exists.
            }
        }

        for(u32 i = 0; i < count; ++i)
        {
            if(pState->loaders[i].id == INVALID_ID){
                pState->loaders[i]      = loader;
                pState->loaders[i].id   = i;
                PINFO("Loader registered!");
                return true;
            }
        }
    }
    return false;
}

bool resourceSystemLoad(const char* name, resourceTypes type, Resource* outResource)
{
    if(pState && type != RESOURCE_TYPE_CUSTOM)
    {
        u32 count = pState->config.maxLoaderCount;
        for(u32 i = 0; i < count; ++i)
        {
            ResourceLoader* loader = &pState->loaders[i];
            if(loader->id != INVALID_ID && pState->loaders[i].type == type) {
                PINFO("Loading %s ...", name);
                return loader->load(loader, name, outResource);
            }
        }
    }
    PWARN("resourceSystemLoad - No loader type found for resource %s.", name);
    return false;
}

bool resourceSystemCustomLoad(const char* name, const char* customType, Resource* outResource)
{
    if(pState)
    {
        u32 count = pState->config.maxLoaderCount;
        for(u32 i = 0; i < count; ++i)
        {
            ResourceLoader* loader = &pState->loaders[i];
            if(loader->id != INVALID_ID && loader->type == RESOURCE_TYPE_CUSTOM && std::strcmp(pState->loaders[i].customType, customType)) {
                return loader->load(loader, name, outResource);
            }
        }
    }
    PWARN("resourceSystemCustomLoad - No loader type found for resource %s.", name);
    return false;
}

void resourceSystemUnload(Resource* resource)
{
    if(pState && resource)
    {
        if(resource->loaderId != INVALID_ID)
        {
            ResourceLoader* l = &pState->loaders[resource->loaderId];
            l->unload(l, resource);
        }
    }
}

// Return the base assets path.
const char* resourceSystemPath(void)
{
    if(pState) {
        return pState->config.assetsBasePath;
    }
    PERROR("Returning an empty string. No assets path provided.");
    return "";
}

bool load(const char* name, ResourceLoader* loader, Resource* outResource)
{
    if(!name || !loader || !loader->load || !outResource) {
        outResource->loaderId = INVALID_ID;
        return false;
    }

    outResource->loaderId = loader->id;
    return loader->load(loader, name, outResource);
}