#pragma once

#include "defines.h"
#include "resources/resourcesTypes.h"

typedef enum DefaultRenderPasses
{
    RENDER_PASS_FORWARD
} DefaultRenderPasses;

typedef struct RenderPacket
{
    f32 deltaTime;

    glm::mat4 model;
    u32 meshesCount;
    Mesh* meshes;
} RenderPacket;
