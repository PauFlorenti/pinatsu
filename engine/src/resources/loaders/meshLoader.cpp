#include "meshLoader.h"

#include "core/logger.h"
#include "core/pstring.h"
#include "platform/filesystem.h"
#include "memory/pmemory.h"

bool meshLoaderLoad(struct resourceLoader* self, const char* name, Resource* outResource)
{
    if(!self || !name || !outResource)
    {
        PERROR("meshLoaderLoad - not enough information provided.");
        return false;
    }

    char fullPath[512];
    const char* format = "%s/%s/%s";
    stringFormat(fullPath, format, resourceSystemPath(), self->typePath, name);
    outResource->path = stringDuplicate(fullPath);

    MeshData* meshResource = (MeshData*)memAllocate(sizeof(MeshData), MEMORY_TAG_ENTITY);

    // Load data from file.
    FileHandle file;
    if(!filesystemOpen(fullPath, FILE_MODE_READ, false, &file)) {
        PERROR("meshLoaderLoad - could not open file %s.", name);
        return false;
    }

    u64 fileSize = 0;
    filesystemSize(&file, &fileSize);
    char buffer[512];
    char* outData = &buffer[0];
    u64 lineLength = 0;
    while(filesystemReadLine(&file, 1024, &lineLength, outData))
    {
        char* trimmed = stringTrim(outData);
        u32 length = stringLength(trimmed);
        u32 index = stringIndexOf(trimmed, ' ');
        char variable[64]; // name variable should not be greater than 64
        memZero(&variable[0], sizeof(char) * 64);
        stringMid(variable, trimmed, 0, index);
        char value[256];    // value should not be greater than 256
        memZero(&value[0], sizeof(char) * 256);
        stringMid(value, trimmed, index, length - index);

        if(stringEquals(variable, "o")) {
            PDEBUG("Name: %s", value);
            stringCopy(value, &meshResource->name[0]);
        }
        else if(stringEquals(variable, "v")) {
        }
        else if(stringEquals(variable, "vt")) {
            PDEBUG("UVs: %s", value);
        }
        else if(stringEquals(variable, "vn")) {
            PDEBUG("Normal: %s", value);
        }
    }

    filesystemClose(&file);

    outResource->name       = name;
    outResource->loaderId   = self->id;
    outResource->dataSize   = fileSize; // data size
    outResource->data       = meshResource;

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
    loader.load         = meshLoaderLoad;
    loader.unload       = meshLoaderUnload;
    loader.customType   = nullptr;
    loader.type         = RESOURCE_TYPE_MESH;
    loader.typePath     = "meshes";
    return loader;
}