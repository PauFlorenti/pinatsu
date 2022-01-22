#pragma once

#include "defines.h"
#include "external/glm/glm.hpp"

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
    u32 loaderId;
    const char* name;
    char* path;
    u64 dataSize;
    void* data;
} Resource;

typedef struct TextureResource
{
    u8* pixels;
    u32 width;
    u32 height;
    u32 channels;
} TextureResource;

#define TEXTURE_NAME_MAX_LENGTH 256

typedef struct Texture
{
    u32 id;
    u32 width;
    u32 height;
    u32 channels;
    bool hasTransparency;
    char name[TEXTURE_NAME_MAX_LENGTH];
    void* data;
} Texture;

#define MATERIAL_NAME_MAX_LENGHT 256

typedef enum MaterialType
{
    MATERIAL_TYPE_FORWARD,
    MATERIAL_TYPE_UI
} MaterialType;

typedef struct MaterialData
{
    char name[MATERIAL_NAME_MAX_LENGHT];
    glm::vec4 diffuseColor;
    MaterialType type;
    char diffuseTextureName[TEXTURE_NAME_MAX_LENGTH];
} MaterialData;

typedef struct Material
{
    u32 id; // Material id
    u32 rendererId; // Used by the renderer to map to its own resources.
    char name[MATERIAL_NAME_MAX_LENGHT];
    MaterialType type;
    glm::vec4 diffuseColor;
    u32 generation; // Check if material has been changed at runtime
    // TODO texture
} Material;

#define MESH_MAX_LENGTH 256

typedef struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 uv;
} Vertex;

typedef struct Mesh {
    u32 id;
    u32 rendererId;
    char name[MESH_MAX_LENGTH];
    Material* material;
} Mesh;

typedef struct MeshData {
    char name[MESH_MAX_LENGTH];
    u32 vertexSize;
    u32 vertexCount;
    Vertex* vertices;
    u32 indexSize;
    u32 indexCount;
    u32* indices;
} MeshData;
