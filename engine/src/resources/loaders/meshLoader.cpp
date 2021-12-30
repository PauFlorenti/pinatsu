#include "meshLoader.h"
#include "core/logger.h"
#include "platform/filesystem.h"

bool meshLoaderLoad(struct resourceLoader* self, const char* name, Resource* outResource)
{
    if(!self || !name || !outResource)
    {
        PERROR("meshLoaderLoad - not enough information provided.");
        return false;
    }

    // Load data from file.
    FileHandle file;
    filesystemOpen(name, FILE_MODE_READ, false, &file);

    u64 fileSize;
    filesystemSize(&file, &fileSize);

    filesystemClose(&file);

    outResource->name       = name;
    outResource->loaderId   = self->id;
    outResource->path       = nullptr;// assets path + name ??
    outResource->dataSize = 0; // data size
    outResource->data = nullptr;

    return true;
}

bool meshLoaderUnload(resourceLoader* self, Resource* resource)
{
    PERROR("meshLoaderUnload - Resource invalid or inexistent.");
    return false;
}

resourceLoader meshLoaderCreate()
{
    resourceLoader loader;
    loader.load = meshLoaderLoad;
    loader.unload = meshLoaderUnload;
    loader.customType = nullptr;
    loader.type = RESOURCE_TYPE_MESH;
    loader.typePath = "";
    return loader;
}