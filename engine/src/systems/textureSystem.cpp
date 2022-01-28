#include "textureSystem.h"

#include "core/logger.h"
#include "core/pstring.h"
#include "memory/pmemory.h"
#include "resources/resourcesTypes.h"
#include "resourceSystem.h"
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

static void textureCreateDefaultTexture();

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
        pState->textures[i].generation = INVALID_ID;
    }

    pState->defaultTexture = (Texture*)memAllocate(sizeof(Texture), MEMORY_TAG_TEXTURE);
    textureCreateDefaultTexture();

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
            textureSystemDestroyTexture(t);
        }
        state = 0;
    }
}

static void textureCreateDefaultTexture()
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
    pState->defaultTexture->id = INVALID_ID;
    pState->defaultTexture->generation = INVALID_ID;

    // TODO renderer create texture
    if(!renderCreateTexture(pixels, pState->defaultTexture, nullptr))
    {
        PFATAL("textureCreateDefaultTexture - failed to create default texture!");
    }
}

Texture* textureSystemGetDefaultTexture()
{
    if(!pState->defaultTexture)
        textureCreateDefaultTexture();
    return pState->defaultTexture;
}

static bool 
loadTexture(const char* name, Texture* t)
{
    Resource txt;
    if(!resourceSystemLoad(name, RESOURCE_TYPE_TEXTURE, &txt)){
        PERROR("loadTexture - Could not load resource '%s'.", name);
        return false;
    }

    TextureResource* textureData = (TextureResource*)txt.data;

    Texture tempTexture;
    tempTexture.width = textureData->width;
    tempTexture.height = textureData->height;
    tempTexture.channels = textureData->channels;

    // Save old generation
    u32 currentGeneration = t->generation;
    t->generation = INVALID_ID;

    u64 totalSize = tempTexture.width * tempTexture.height * tempTexture.channels;
    bool hasTransparency = false;
    for(u64 i = 0; i < totalSize; i += tempTexture.channels)
    {
        if(textureData->pixels[i + 3] < 255){
            hasTransparency = true;
            break;
        }
    }

    tempTexture.hasTransparency = hasTransparency;
    tempTexture.generation = INVALID_ID;
    stringCopy(name, tempTexture.name);

    renderCreateTexture(textureData->pixels, &tempTexture, &txt);

    Texture oldTexture = *t;
    *t = tempTexture;
    t->id = oldTexture.id;

    renderDestroyTexture(&oldTexture);
    
    // Assign the generation
    if(currentGeneration == INVALID_ID){
        t->generation = 0;
    } else {
        t->generation = currentGeneration + 1;
    }

    resourceSystemUnload(&txt);
    return true;

}

Texture* 
textureSystemGet(const char* name)
{
    if(stringEquals(name, DEFAULT_TEXTURE_NAME)){
        return pState->defaultTexture;
    }

    for(u32 i = 0; i < pState->config.maxTextureCount; ++i)
    {
        if(stringEquals(pState->textures[i].name, name)){
            return &pState->textures[i];
        }
    }

    Texture* t = 0;
    for (u32 i = 0; i < pState->config.maxTextureCount; ++i)
    {
        if (pState->textures[i].id == INVALID_ID) {
            t = &pState->textures[i];
            t->id = i;
            break;
        }
    }
    // If texture was not there, load it
    if(!loadTexture(name, t)){
        PERROR("textureSystemGet - Could not load texture '%s'.", name);
        return nullptr;
    }

    return t;
}

void 
textureSystemRelease(const char* name)
{
    if(stringEquals(name, DEFAULT_TEXTURE_NAME)){
        return;
    }

    Texture* t;
    for(u32 i = 0; i < pState->config.maxTextureCount; i++){
        if(stringEquals(pState->textures[i].name, name)){
            t = &pState->textures[i];
            break;
        }
    }

    textureSystemDestroyTexture(t);
}

void 
textureSystemDestroyTexture(Texture* t)
{
    renderDestroyTexture(t);

    memZero(t->name, sizeof(char) * TEXTURE_NAME_MAX_LENGTH);
    memZero(t, sizeof(Texture));
    t->id = INVALID_ID;
    t->generation = INVALID_ID;
}