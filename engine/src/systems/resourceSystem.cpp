#include "resourceSystem.h"

#include "core/logger.h"
#include "resources/loaders/meshLoader.h"
#include "resources/loaders/textureLoader.h"

// TODO Make own string container funcs.
#include <cstring>

typedef struct ResoureSystemState{
    resourceSystemConfig config;
    resourceLoader* loaders;
} ResourceSystemState;

static ResourceSystemState* pState = nullptr;

bool resourceSystemInit(u64* memoryRequirements, void* state, resourceSystemConfig config)
{
    // If system not init yet, return memory requirements to be initialized.
    if(state == nullptr)
    {
        *memoryRequirements = sizeof(ResourceSystemState) + (sizeof(resourceLoader) * config.maxLoaderCount);
        return true;
    }

    // Check the config is valid.
    if(config.maxLoaderCount < 1){
        PERROR("Maximum loaders allowed is below 1. That is an error, shutting down.");
        return false;
    }
    if(!config.assetsBasePath) {
        PERROR("resourceSystemInit - No assets base path has been given. Shutting down.");
        return false;
    }

    pState = static_cast<ResourceSystemState*>(state);
    pState->config = config;

    void* resourceLoaderPtr = pState + sizeof(resourceSystemConfig);
    //pState->loaders = static_cast<resourceLoader*>(pState + sizeof(resourceSystemConfig));
    pState->loaders = (resourceLoader*)(pState + sizeof(pState->config));

    for(u32 i = 0; i < pState->config.maxLoaderCount; i++)
    {
        pState->loaders[i].id = INVALID_ID;
    }

    resourceSystemRegisterLoader(meshLoaderCreate());

    PINFO("Resource system initialized with base path %s.", config.assetsBasePath);

    return true;
}

void resourceSystemShutdown(void* state)
{
    if(state)
        state = nullptr;
}

bool resourceSystemRegisterLoader(const resourceLoader& loader)
{
    if(pState)
    {
        u32 count = pState->config.maxLoaderCount;
        // Make sure resource loader is not already registered
        for(u32 i = 0; i < count; i++)
        {
            resourceLoader* l = &pState->loaders[i];
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
            resourceLoader* loader = &pState->loaders[i];
            PINFO("Loading %s ...", name);
            if(loader->id != INVALID_ID && pState->loaders[i].type == type) {
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
            resourceLoader* loader = &pState->loaders[i];
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
            resourceLoader* l = &pState->loaders[resource->loaderId];
            l->unload(l, resource);
        }
    }
}

const char* resourceSystemPath(void)
{
    if(pState) {
        return pState->config.assetsBasePath;
    }
    PERROR("Returning an empty string. No assets path provided.");
    return "";
}

bool load(const char* name, resourceLoader* loader, Resource* outResource)
{
    if(!name || !loader || !loader->load || !outResource) {
        outResource->loaderId = INVALID_ID;
        return false;
    }

    outResource->loaderId = loader->id;
    return loader->load(loader, name, outResource);
}