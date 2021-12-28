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

    return true;
}

void meshSystemShutdown(void* state)
{
    if(state) {
        pState = nullptr;
    }
}

static void meshSystemSetMesh(Mesh* mesh)
{
    for(u32 i = 0; i < pState->config.maxMeshesCount; ++i)
    {
        if(pState->meshes[i].id == mesh->id){
            PWARN("Mesh already set in system.");
            return;
        }
        if(pState->meshes[i].id == INVALID_ID)
        {
            pState->meshes[i] = *mesh;
            pState->meshes[i].id = i;
            return;
        }
    }
}

Mesh* meshSystemGetPlane(u32 width, u32 height)
{
    MeshData* mesh;
    Mesh* meshref;

    mesh->vertexCount = 6;
    mesh->vertices = (Vertex*)memAllocate(sizeof(Vertex) * mesh->vertexCount, MEMORY_TAG_UNKNOWN);
    //mesh.name = "Plane";
    mesh->indices = nullptr;
    mesh->indexCount = 0;

    mesh->vertices[0] = {{-0.5f, -0.5f,  0.5f}, glm::vec3(1)};
    mesh->vertices[1] = {{ 0.5f, -0.5f,  0.5f}, glm::vec3(1)};
    mesh->vertices[2] = {{-0.5f,  0.5f,  0.5f}, glm::vec3(1)};

    mesh->vertices[4] = {{ 0.5f,  0.5f,  0.5f}, glm::vec3(1)};
    mesh->vertices[3] = {{ 0.5f, -0.5f,  0.5f}, glm::vec3(1)};
    mesh->vertices[5] = {{-0.5f,  0.5f,  0.5f}, glm::vec3(1)};

    //meshSystemSetMesh(mesh);

    return meshref;
}