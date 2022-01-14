#include "meshSystem.h"

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
        if(mesh->id != INVALID_ID && pState->meshes[i].id == mesh->id){
            PWARN("Mesh already set in system.");
            return;
        }
        if(pState->meshes[i].id == INVALID_ID)
        {
            pState->meshes[i] = *mesh;
            pState->meshes[i].id = i;
            return;
        }
        if(i == pState->meshCount - 1) {
            PERROR("meshSystemSetMesh - No room for a new mesh.");
        }
    }
}

// TODO  add the functionality to create segments and make it more customizable
Mesh* meshSystemGetPlane(u32 width, u32 height)
{
    MeshData* mesh = (MeshData*)memAllocate(sizeof(MeshData), MEMORY_TAG_ENTITY);
    Mesh* meshref = (Mesh*)memAllocate(sizeof(Mesh), MEMORY_TAG_ENTITY);
    meshref->id = INVALID_ID;
    char* name = "Plane";
    std::memcpy(&meshref->name, name, 5);

    mesh->vertexCount = 6;
    mesh->vertices = (Vertex*)memAllocate(sizeof(Vertex) * mesh->vertexCount, MEMORY_TAG_UNKNOWN);
    std::memcpy(&mesh->name, name, 5);
    mesh->indices = nullptr;
    mesh->indexCount = 0;

    mesh->vertices[0] = {{-0.5f, -0.5f,  0.5f}, glm::vec4(1), {0, 0}};
    mesh->vertices[1] = {{ 0.5f, -0.5f,  0.5f}, glm::vec4(1), {1, 0}};
    mesh->vertices[2] = {{-0.5f,  0.5f,  0.5f}, glm::vec4(1), {0, 1}};

    mesh->vertices[4] = {{ 0.5f,  0.5f,  0.5f}, glm::vec4(1), {1, 1}};
    mesh->vertices[3] = {{ 0.5f, -0.5f,  0.5f}, glm::vec4(1), {1, 0}};
    mesh->vertices[5] = {{-0.5f,  0.5f,  0.5f}, glm::vec4(1), {0, 1}};

    meshSystemSetMesh(meshref);
    renderCreateMesh(meshref, mesh->vertexCount, mesh->vertices, 0, nullptr);

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