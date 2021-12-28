#include "meshLoader.h"
#include "core/logger.h"

typedef struct MeshLoaderState
{
    u32 maxMeshesCount;
    Mesh* registeredMeshes;
} MeshLoaderState;

static MeshLoaderState* pState;

bool meshLoaderLoad(struct resourceLoader* self, const char* name, Resource* outResource)
{
    if(!self || !name || !outResource)
    {
        return false;
    }

    // check if resource already loaded.
    if(outResource && outResource->id)
    {
        for(u32 i = 0; i < pState->maxMeshesCount; ++i){
            if(pState->registeredMeshes[i].id == outResource->id)
            {
                PINFO("Mesh %s already loaded.", name);
                return true;
            }
        }
    }

    outResource->name = name;
    outResource->loaderId = self->id;
    outResource->path = nullptr;// assets path + name ??
    
    for(u32 i = 0; i < pState->maxMeshesCount; ++i){
        if(pState->registeredMeshes[i].id == INVALID_ID)
        {
            outResource->id = i;
            return true;
        }
    }


    return true;
}

void meshLoaderUnload(Resource* resource)
{
    if(resource && resource->id != INVALID_ID)
    {
        for(u32 i = 0; i < pState->maxMeshesCount; ++i)
        {
            if(pState->registeredMeshes[i].id == resource->id){
                pState->registeredMeshes[i].id = INVALID_ID;
                return;
            }
        }
    }
}