#include "meshSystem.h"

#include "defines.h"
#include "core/logger.h"
#include "memory/pmemory.h"
#include "renderer/rendererFrontend.h"

// TODO make own library
//#include "math_types.h"
#include "external/glm/gtc/constants.hpp"
#include "external/glm/glm.hpp"

typedef struct MeshSystemState
{
    MeshSystemConfig config;
    u32 meshCount;
    Mesh* meshes;
} MeshSystemState;

static MeshSystemState* pState;

bool meshSystemInit(u64* memoryRequirements, void* state, MeshSystemConfig configuration)
{
    u64 stateMemoryRequirement = sizeof(MeshSystemState);
    u64 meshesMemoryRequirement = sizeof(Mesh) * configuration.maxMeshesCount;

    if(state == nullptr) {
        *memoryRequirements = stateMemoryRequirement + meshesMemoryRequirement;
        return true;
    }

    pState = static_cast<MeshSystemState*>(state);
    pState->config = configuration;
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

    u32 i[3];
    memZero(i, sizeof(u32) * 3);
    i[0] = 0;
    i[1] = 1;
    i[2] = 2;

    meshSystemSetMesh(m);
    if(!renderCreateMesh(m, 3, v, 3, i)){
        return nullptr;
    }

    return m;
}

// TODO  make it configurable. Currently drawing at length 1.
Mesh* meshSystemGetPlane(u32 width, u32 height)
{
    Mesh* m = (Mesh*)memAllocate(sizeof(Mesh), MEMORY_TAG_ENTITY);
    m->id = INVALID_ID;
    m->rendererId = INVALID_ID;

    u32 vertexSize = sizeof(Vertex);
    u32 indexSize = sizeof(u32);

    Vertex v[4];
    memZero(v, vertexSize * 4);

    v[0].position = {-1.0f * (f32)width, -1.0f * (f32)height, 0.0f};
    v[1].position = { 1.0f * (f32)width, -1.0f * (f32)height, 0.0f};
    v[2].position = {-1.0f * (f32)width,  1.0f * (f32)height, 0.0f};
    v[3].position = { 1.0f * (f32)width,  1.0f * (f32)height, 0.0f};

    v[0].color = glm::vec4(1);
    v[1].color = glm::vec4(1);
    v[2].color = glm::vec4(1);
    v[3].color = glm::vec4(1);

    v[0].uv = glm::vec2(0, 0);
    v[1].uv = glm::vec2(1, 0);
    v[2].uv = glm::vec2(0, 1);
    v[3].uv = glm::vec2(1, 1);

    u32 i[6] = {0, 1, 2, 1, 3, 2};
    
    meshSystemSetMesh(m);
    if(!renderCreateMesh(m, 4, v, 4, i)){
        return nullptr;
    }

    return m;
}

// TODO make it configurable. Currently drawing at length 1 and 12 segments.
// TODO fix circle creation.
Mesh* meshSystemGetCircle(f32 r)
{
    Mesh* m = (Mesh*)memAllocate(sizeof(Mesh), MEMORY_TAG_ENTITY);
    m->id = INVALID_ID;
    m->rendererId = INVALID_ID;

    const u32 nSegments = 6;
    const f32 radius = 1.0f;
    const u32 nVertices = nSegments + 1; // Include the center vertex

    Vertex v[nVertices];
    memZero(v, sizeof(Vertex) * nVertices);

    f32 angleDeg = (2.0f * glm::pi<f32>() * radius) / nSegments;
    f32 angleRad = angleDeg / 180.0f;
    for(u32 i = 0; i < nSegments; ++i)
    {
        f32 x = cos(i * angleRad) * radius;
        f32 y = sin(i * angleRad) * radius;
        v[i].position = {x, y, 0.0f};
        v[i].color = glm::vec4(1);
    }
    v[nVertices - 1].position = {0.0f, 0.0f, 0.0f};
    v[nVertices - 1].color = glm::vec4(1);

    u32 i[18] = { 0, 1, 7, 
                  1, 2, 7,
                  2, 3, 7,
                  3, 4, 7,
                  4, 5, 7,
                  5, 6, 7 };

    meshSystemSetMesh(m);
    if(!renderCreateMesh(m, nVertices, v, 18, i)){
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