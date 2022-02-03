#include "meshLoader.h"

#include "core/logger.h"
#include "core/pstring.h"
#include "platform/filesystem.h"
#include "memory/pmemory.h"
#include "memory/linearAllocator.h"

bool meshLoaderLoad(struct ResourceLoader* self, const char* name, Resource* outResource)
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

    LinearAllocator alloc;
    linearAllocatorCreate(sizeof(u32) * 1024, nullptr, &alloc);

    glm::vec3* vertices = (glm::vec3*)linearAllocatorAllocate(&alloc, 1024);
    u32 nVertices = 0;
    glm::vec2* uvs = (glm::vec2*)linearAllocatorAllocate(&alloc, 1024);
    u32 nUvs = 0;
    glm::vec3* normals = (glm::vec3*)linearAllocatorAllocate(&alloc, 1024);
    u32 nNormals = 0;
    glm::vec3* faces = (glm::vec3*)linearAllocatorAllocate(&alloc, 1024);
    u32 nFaces = 0;

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
        stringMid(value, trimmed, index + 1, length - index);

        if(stringEquals(variable, "o")) {
            stringCopy(value, &meshResource->name[0]);
        }
        else if(stringEquals(variable, "v")) {
            glm::vec3 v;
            stringToVec3(value, &v);
            vertices[nVertices] = v;
            nVertices++;
        }
        else if(stringEquals(variable, "vt")) {
            glm::vec2 v;
            stringToVec2(value, &v);
            uvs[nUvs] = v;
            nUvs++;
        }
        else if(stringEquals(variable, "vn")) {
            glm::vec3 v;
            stringToVec3(value, &v);
            normals[nNormals] = v;
            nNormals++;
        }
        else if(stringEquals(variable, "f")) {
            char str1[6];
            char str2[6];
            char str3[6];
            sscanf(value, "%s %s %s", &str1, &str2, &str3);
            stringToObjFace(&str1[0], &faces[nFaces]);
            nFaces++;
            stringToObjFace(&str2[0], &faces[nFaces]);
            nFaces++;
            stringToObjFace(&str3[0], &faces[nFaces]);
            nFaces++;
        }
    }

    meshResource->vertices = (Vertex*)memAllocate(sizeof(Vertex) * nFaces, MEMORY_TAG_ENTITY);
    meshResource->vertexCount = nFaces;
    for(u32 i = 0; i < nFaces; ++i) {
        meshResource->vertices[i].position  = vertices[(u32)faces[i].x - 1];
        meshResource->vertices[i].color     = glm::vec4(normals[(u32)faces[i].z - 1], 1);
        meshResource->vertices[i].uv        = uvs[(u32)faces[i].y - 1];
    }

    linearAllocatorDestroy(&alloc);
    filesystemClose(&file);

    outResource->name       = name;
    outResource->loaderId   = self->id;
    outResource->dataSize   = sizeof(MeshData);
    outResource->data       = meshResource;

    return true;
}

bool meshLoaderUnload(ResourceLoader* self, Resource* resource)
{
    PERROR("meshLoaderUnload - Resource invalid or inexistent.");
    return false;
}

ResourceLoader meshLoaderCreate()
{
    ResourceLoader loader;
    loader.id           = INVALID_ID;
    loader.load         = meshLoaderLoad;
    loader.unload       = meshLoaderUnload;
    loader.customType   = nullptr;
    loader.type         = RESOURCE_TYPE_MESH;
    loader.typePath     = "meshes";
    return loader;
}