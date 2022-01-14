#pragma once

#include "defines.h"
#include "resources/resourcesTypes.h"

typedef struct TextureSystemConfig
{
    u32 maxTextureCount;
} TextureSystemConfig;

bool textureSystemInit(u64* memoryRequirements, void* state, TextureSystemConfig config);
void textureSystemShutdown(void* state);

Texture* textureSystemGetTexture(const char* name);
Texture* textureSystemGetDefaultTexture();

