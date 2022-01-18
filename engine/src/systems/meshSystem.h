#pragma once

#include "defines.h"
#include "resources/resourcesTypes.h"

typedef struct MeshSystemConfig
{
    u32 maxMeshesCount;
} MeshSystemConfig;

bool meshSystemInit(u64* memoryRequirements, void* state, MeshSystemConfig config);
void meshSystemShutdown(void* state);

// TODO make with segments, more custom.
Mesh* meshSystemGetTriangle();
//Mesh* meshSystemGetPlane(u32 width, u32 height);
Mesh* meshSystemCreateFromData(const MeshData* data);