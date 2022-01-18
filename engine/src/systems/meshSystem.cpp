#include "meshSystem.h"

#include "defines.h"
#include "core/logger.h"
#include "memory/pmemory.h"
#include "external/glm/glm.hpp"
#include "renderer/rendererFrontend.h"

typedef struct MeshSystemState
{
    MeshSystemConfig config;
    u32 meshCount;
    Mesh* meshes;
} MeshSystemState;

static MeshSystemState* pState;

bool meshSystemInit(u64* memoryRequirements, void* state, MeshSystemConfig config)
{
    u64 stateMemoryRequirement = sizeof(MeshSystemState);
    u64 meshesMemoryRequirement = sizeof(Mesh) * config.maxMeshesCount;
    *memoryRequirements = stateMemoryRequirement + meshesMemoryRequirement;

    if(!state) {
        return true;
    }

    pState = static_cast<MeshSystemState*>(state);
    pState->config = config;
    pState->meshes = (Mesh*)(pState + stateMemoryRequirement);

    for(u32 i = 0; i < pState->config.maxMeshesCount; ++i) {
        pState->meshes[i].id = INVALID_ID;
    }

    PINFO("Mesh system initialized!");
    return true;
}

void meshSystemShutdown(void* state)
{
    if(state) {
        // TODO render destroy mesh. Free GPU memory for all updated meshes.
        pState = nullptr;
    }
}

static void meshSystemSetMesh(Mesh* mesh)
{
    for(u32 i = 0; i < pState->config.maxMeshesCount; ++i)
    {
        if(pState->meshes[i].id == INVALID_ID)
        {
            pState->meshes[i] = *mesh;
            pState->meshes[i].id = i;
            mesh->id = i;
            pState->meshCount++;
            return;
        }
        if(mesh->id != INVALID_ID && pState->meshes[i].id == mesh->id){
            PWARN("Mesh already set in system.");
            return;
        }
    }
}

Mesh* meshSystemGetTriangle()
{
    MeshData* meshData = (MeshData*)memAllocate(sizeof(MeshData), MEMORY_TAG_ENTITY);
    Mesh* mesh = (Mesh*)memAllocate(sizeof(Mesh*), MEMORY_TAG_ENTITY);
    mesh->id = INVALID_ID;
    mesh->rendererId = INVALID_ID;
    char* name = "Triangle";
    std::memcpy(&meshData->name, name, 8);
    std::memcpy(&mesh->name, name, 8);

    meshData->vertexCount = 3;
    meshData->vertices = (Vertex*)memAllocate(sizeof(Vertex) * meshData->vertexCount, MEMORY_TAG_ENTITY);

    meshData->vertices[0] = {{-0.5f, -0.5f, 0.0f}, glm::vec4(1), {0, 0}};
    meshData->vertices[1] = {{ 0.5f, -0.5f, 0.0f}, glm::vec4(1), {0, 0}};
    meshData->vertices[2] = {{ 0.0f,  0.5f, 0.0f}, glm::vec4(1), {0, 0}};

    //u32 index[3] = {0, 1, 2};

    meshSystemSetMesh(mesh);
    renderCreateMesh(mesh, meshData->vertexCount, meshData->vertices, 0, nullptr);
    return mesh;
}

// TODO  add the functionality to create segments and make it more customizable
Mesh* meshSystemGetPlane(u32 width, u32 height)
{
    MeshData* mesh = (MeshData*)memAllocate(sizeof(MeshData), MEMORY_TAG_ENTITY);
    Mesh* meshref = (Mesh*)memAllocate(sizeof(Mesh), MEMORY_TAG_ENTITY);
    meshref->id = INVALID_ID;
    char* name = "Plane";
    std::memcpy(&meshref->name, name, 5);

    std::memcpy(&mesh->name, name, 5);
    mesh->vertexCount = 4;
    mesh->vertices = (Vertex*)memAllocate(sizeof(Vertex) * mesh->vertexCount, MEMORY_TAG_UNKNOWN);

    mesh->vertices[0] = {{-0.5f, -0.5f,  0.0f}, glm::vec4(1), {0, 0}};
    mesh->vertices[1] = {{ 0.5f, -0.5f,  0.0f}, glm::vec4(1), {0, 0}};
    mesh->vertices[2] = {{-0.5f,  0.5f,  0.0f}, glm::vec4(1), {0, 0}};
    mesh->vertices[4] = {{ 0.5f,  0.5f,  0.0f}, glm::vec4(1), {0, 0}};

    u32 array[3] = {1, 2, 3};
    mesh->indices = array;
    mesh->indexCount = 3;

    meshSystemSetMesh(meshref);
    renderCreateMesh(meshref, mesh->vertexCount, mesh->vertices, mesh->indexCount, mesh->indices);

    return meshref;
}

Mesh* meshSystemCreateFromData(const MeshData* data)
{
    if(!data) {
        return false;
    }

    // TODO make sure mesh is not already updated.

    Mesh* mesh = (Mesh*)memAllocate(sizeof(Mesh), MEMORY_TAG_ENTITY);
    mesh->id = INVALID_ID;
    
    meshSystemSetMesh(mesh);
    if(!renderCreateMesh(mesh, data->vertexCount, data->vertices, data->indexCount, data->indices)) {
        PERROR("meshSystemCreateFromData - Error al create mesh in renderer.");
    }
    return mesh;
}