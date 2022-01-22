#pragma once

#include "defines.h"
#include "resources/resourcesTypes.h"

typedef struct MaterialSystemConfig
{
    u32 maxMaterialCount;
} MaterialSystemConfig;

bool materialSystemInit(u64* memoryRequirements, void* state, MaterialSystemConfig config);
void materialSystemShutdown(void* state);

Material* materialSystemCreateFromData(MaterialData data);
Material* materialSystemGetMaterialByName(const char* name);
void materialSystemCreateDefaultMaterial();