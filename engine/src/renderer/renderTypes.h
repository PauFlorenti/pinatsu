#pragma once

#include "defines.h"
#include "resources/resourcesTypes.h"

// TEMP
#include "external/glm/glm.hpp"

typedef enum DefaultRenderPasses
{
    RENDER_PASS_FORWARD
} DefaultRenderPasses;

typedef struct RenderMeshData
{
    glm::mat4 model;
    Mesh* mesh;
} RenderMeshData;

typedef struct RenderPacket
{
    f32 deltaTime;

    u32 renderMeshDataCount;
    RenderMeshData* meshes;
} RenderPacket;
