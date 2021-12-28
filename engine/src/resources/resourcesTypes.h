#pragma once

#include "defines.h"

typedef enum resourceTypes
{
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_TEXTURE,
    RESOURCE_TYPE_MESH,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_CUSTOM
} resourceTypes;

typedef struct Resource
{
    u32 id;
    u32 loaderId;
    const char* name;
    char* path;
    u64 dataSize;
    void* data;
} Resource;

typedef struct Texture
{
    u32 id;
    u32 width;
    u32 height;
} Texture;

typedef struct Material
{
    u32 id;
    const char* name;
} Material;

#define MESH_MAX_LENGTH 256

typedef struct Mesh {
    u32 id;
    char name[MESH_MAX_LENGTH];
    Material* material;
} Mesh;