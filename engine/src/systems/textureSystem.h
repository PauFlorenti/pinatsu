#pragma once

#include "defines.h"
#include "resources/resourcesTypes.h"

#define DEFAULT_TEXTURE_NAME "default"

typedef struct TextureSystemConfig
{
    u32 maxTextureCount;
} TextureSystemConfig;

bool 
textureSystemInit(u64* memoryRequirements, void* state, TextureSystemConfig config);

void 
textureSystemShutdown(void* state);

Texture* 
textureSystemGet(const char* name, bool autoRelease = false);

Texture* 
textureSystemGetDefaultTexture();

void 
textureSystemRelease(const char* name);