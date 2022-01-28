#include "textureLoader.h"

#include "core/logger.h"
#include "core/pstring.h"
#include "memory/pmemory.h"
#include "platform/filesystem.h"
#include "systems/resourceSystem.h"
#include "resources/resourcesTypes.h"

#define STB_IMAGE_IMPLEMENTATION
#include <external/stb/stb_image.h>

bool textureLoaderLoad(struct ResourceLoader* self, const char* name, Resource* outResource)
{
    if(!self || !name || !outResource)
    {
        PERROR("textureLoaderLoad - Unable to load the resource.");
        return false;
    }

    char fullPath[512];
    const char* format = "%s/%s/%s";
    stringFormat(&fullPath[0], format, resourceSystemPath(), "textures", name);
    outResource->path = fullPath;

    TextureResource* textureResource = (TextureResource*)memAllocate(sizeof(TextureResource), MEMORY_TAG_TEXTURE);

    FileHandle handle;
    if(!filesystemOpen(fullPath, FILE_MODE_READ, false, &handle))
    {
        PERROR("Could not open file %s.", fullPath);
        return false;
    }

    const i32 requiredChannels = 4;
    i32 width = 0, height = 0, channels = 0;
    u8* data = stbi_load(fullPath, &width, &height, &channels, requiredChannels);

    const char* fail = stbi_failure_reason();
    if(fail) {
        PERROR("textureLoaderLoad - Texture resource failed to load file '%s' : %s.", fullPath, fail);
        if(data) {
            stbi_image_free(data);
        }
        return false;
    }

    if(!data) {
        PERROR("textureLoaderLoad - Texture resource failed to load file '%s'.", fullPath);
        return false;
    }

    textureResource->pixels     = data;
    textureResource->width      = width;
    textureResource->height     = height;
    textureResource->channels   = requiredChannels;

    outResource->dataSize   = sizeof(TextureResource);
    outResource->data       = (void*)textureResource;
    outResource->loaderId   = self->id;
    outResource->name       = name;

    filesystemClose(&handle);
    return true;
}

bool textureLoaderUnload(ResourceLoader* self, Resource* resource)
{
    if(!self || !resource){
        PWARN("textureLoaderUnload - called with self or resource as nullptr.");
        return false;
    }

    u32 length = stringLength(resource->path);
    if(length){
        //memFree(resource->path, sizeof(char) * length, MEMORY_TAG_STRING);
    }

    if(resource->data){
        memFree(resource->data, resource->dataSize, MEMORY_TAG_TEXTURE);
        resource->dataSize = 0;
        resource->data = nullptr;
        resource->loaderId = INVALID_ID;
    }
    return true;
}

ResourceLoader textureLoaderCreate()
{
    ResourceLoader resource;
    resource.id = INVALID_ID;
    resource.load = textureLoaderLoad;
    resource.unload = textureLoaderUnload;
    resource.type = RESOURCE_TYPE_TEXTURE;
    resource.typePath = "textures";
    resource.customType = nullptr;
    return resource;
}