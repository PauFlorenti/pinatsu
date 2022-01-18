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

    Mesh* m = (Mesh*)memAllocate(sizeof(Mesh), MEMORY_TAG_ENTITY);
    m->id = INVALID_ID;
    m->rendererId = INVALID_ID;

    Vertex v[3];
    memZero(v, sizeof(Vertex) * 3);

    v[0].position = {-0.5f, -0.5f, 0.0f};
    v[1].position = { 0.5f, -0.5f, 0.0f};
    v[2].position = { 0.0f,  0.5f, 0.0f};

    v[0].color = glm::vec4(1);
    v[1].color = glm::vec4(1);
    v[2].color = glm::vec4(1);

    meshSystemSetMesh(m);
    if(!renderCreateMesh(m, 3, v, 0, nullptr)){
        return nullptr;
    }

    return m;
}

Mesh* meshSystemCreateFromData(const MeshData* data)
{
    if(!data) {
        return false;
    }

    // TODO make sure mesh is not already updated.

    Mesh* mesh = (Mesh*)memAllocate(sizeof(Mesh), MEMORY_TAG_ENTITY);
    mesh->id = INVALID_ID;
    mesh->rendererId = INVALID_ID;
    
    meshSystemSetMesh(mesh);
    if(!renderCreateMesh(mesh, data->vertexCount, data->vertices, data->indexCount, data->indices)) {
        PERROR("meshSystemCreateFromData - Error al create mesh in renderer.");
    }
    return mesh;
}