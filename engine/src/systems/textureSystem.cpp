#include "textureSystem.h"

#include "core/logger.h"
#include "memory/pmemory.h"
#include "resources/resourcesTypes.h"
#include "renderer/rendererFrontend.h"

typedef struct TextureSystemState
{
    // * TEMP
    Texture* defaultTexture;
    
    TextureSystemConfig config;
    u32 textureCount;
    Texture* textures;
} TextureSystemState;

static TextureSystemState* pState;

bool textureSystemInit(u64* memoryRequirements, void* state, TextureSystemConfig config)
{
    if(config.maxTextureCount < 1)
    {
        PERROR("textureSystemInit - Config received is not valid. Cannot initialize the texture system.");
        return false;
    }

    u64 stateMemoryRequirements = sizeof(TextureSystemState);
    u64 resourcesMemoryRequirements = sizeof(TextureResource) * config.maxTextureCount;
    *memoryRequirements = stateMemoryRequirements + resourcesMemoryRequirements;

    if(!state) {
        return true;
    }

    pState = (TextureSystemState*)state;
    pState->config = config;
    pState->textures = (Texture*)(pState + stateMemoryRequirements);

    for(u32 i = 0;
        i < pState->config.maxTextureCount;
        ++i)
    {
        pState->textures[i].id = INVALID_ID;
    }

    pState->defaultTexture = (Texture*)memAllocate(sizeof(Texture*), MEMORY_TAG_TEXTURE);
    pState->defaultTexture->id = INVALID_ID;
    // TODO create default texture.

    return true;
}

void textureSystemShutdown(void* state)
{
    if(pState)
    {
        for(u32 i = 0;
            i < pState->config.maxTextureCount;
            ++i)
        {
            Texture* t = &pState->textures[i];
            // TODO renderDestroyTexture - free all GPU memory from updated textures.
        }
        state = 0;
    }
}

Texture* textureSystemGetDefaultTexture()
{
    // Create a default 256x256 texture with a checkered pattern.
    PDEBUG("Creating default texture.");
    const u32 textureDimension = 256;
    const u32 channels = 4;
    const u32 pixelCount = textureDimension * textureDimension;
    u8 pixels[pixelCount * channels];
    memSet(&pixels, 255, sizeof(u8) * pixelCount * channels);
    
    for(u64 row = 0; row < textureDimension; ++row)
    {
        for(u64 col = 0; col < textureDimension; ++col)
        {
            u64 index = (row * textureDimension) + col;
            u64 index_bpp = index * channels;
            if(row % 2)
            {
                if(col % 2)
                {
                    pixels[index_bpp] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            } else {
                if(!(col % 2)) 
                {
                    pixels[index_bpp] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            }
        }
    }

    pState->defaultTexture->width = textureDimension;
    pState->defaultTexture->height = textureDimension;
    pState->defaultTexture->hasTransparency = false;
    pState->defaultTexture->channels = 4;
    pState->defaultTexture->data = &pixels;
    pState->defaultTexture->id = INVALID_ID;

    // TODO renderer create texture
    renderCreateTexture(pixels, pState->defaultTexture);

    return pState->defaultTexture;
}