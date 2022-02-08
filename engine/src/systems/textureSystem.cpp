#include "textureSystem.h"

#include "core/logger.h"
#include "core/pstring.h"
#include "memory/pmemory.h"
#include "resources/resourcesTypes.h"
#include "resourceSystem.h"
#include "renderer/rendererFrontend.h"
#include "containers/hashtable.h"

#include <unordered_map>

struct TextureReference
{
    u32 referenceCount;
    u32 handle;
    bool autoRelease;
};

struct TextureSystemState
{
    // * TEMP
    Texture* defaultTexture;
    
    TextureSystemConfig config;
    //u32 textureCount;
    Texture* textures;
    Hashtable hashtable;  // Change name
    //std::unordered_map<std::string, TextureReference> map;
};

static TextureSystemState* pState;

static void 
textureCreateDefaultTexture()
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

    if(!renderCreateTexture(pixels, pState->defaultTexture))
    {
        PFATAL("textureCreateDefaultTexture - failed to create default texture!");
    }
}

static void 
textureSystemDestroyTexture(Texture* t)
{
    renderDestroyTexture(t);

    memZero(t->name, sizeof(char) * TEXTURE_NAME_MAX_LENGTH);
    memZero(t, sizeof(Texture));
    t->id = INVALID_ID;
    t->generation = INVALID_ID;
}

static void 
textureCreateBasicTextures()
{
    // White texture
    const u32 textureDimension = 256;
    const u32 channels = 4;
    const u32 pixelCount = textureDimension * textureDimension;
    u8 pixels[pixelCount * channels];
    // Write white pixels
    memSet(&pixels, 255, sizeof(u8) * pixelCount * channels);

    Texture* t;
    for(u32 i = 0; i < pState->config.maxTextureCount; i++){
        if(pState->textures[i].id == INVALID_ID){
            t = &pState->textures[i];
            t->id = i;
            break;
        }
    }

    t->width = textureDimension;
    t->height = textureDimension;
    t->channels = channels;
    t->hasTransparency = false;
    stringCopy("white", t->name);
    if(!renderCreateTexture(pixels, t)){
        PERROR("textureCreateBasicTextures - renderer could not create the texture.");
    }

}

bool 
textureSystemInit(u64* memoryRequirements, void* state, TextureSystemConfig config)
{
    if(config.maxTextureCount < 1)
    {
        PERROR("textureSystemInit - Config received is not valid. Cannot initialize the texture system.");
        return false;
    }

    u64 stateMemoryRequirements = sizeof(TextureSystemState);
    u64 arrayMemoryRequirements = sizeof(Texture) * config.maxTextureCount;
    u64 hashtableMemoryRequirements = sizeof(TextureReference) * config.maxTextureCount;
    *memoryRequirements = stateMemoryRequirements + arrayMemoryRequirements + hashtableMemoryRequirements;

    if(!state) {
        return true;
    }

    pState = (TextureSystemState*)state;
    pState->config = config;
    pState->textures = (Texture*)(pState + stateMemoryRequirements);

    void* hashtableMemoryBlock = pState->textures + arrayMemoryRequirements;

    // Create hashtable
    hashtableCreate(sizeof(TextureReference), config.maxTextureCount, hashtableMemoryBlock, &pState->hashtable);

    // Fill hashtable with invalid references
    TextureReference invalidReference{};
    invalidReference.autoRelease = false;
    invalidReference.referenceCount = 0;
    invalidReference.handle = INVALID_ID;
    hashtableFill(&pState->hashtable, &invalidReference);
    //pState->map.reserve(config.maxTextureCount);

    for(u32 i = 0;
        i < pState->config.maxTextureCount;
        ++i)
    {
        pState->textures[i].id = INVALID_ID;
        pState->textures[i].generation = INVALID_ID;
    }

    pState->defaultTexture = (Texture*)memAllocate(sizeof(Texture), MEMORY_TAG_TEXTURE);
    textureCreateDefaultTexture();
    textureCreateBasicTextures();

    return true;
}

void 
textureSystemShutdown(void* state)
{
    if(pState)
    {
        textureSystemDestroyTexture(pState->defaultTexture);
        for(u32 i = 0;
            i < pState->config.maxTextureCount;
            ++i)
        {
            Texture* t = &pState->textures[i];
            if(t->id != INVALID_ID)
                textureSystemDestroyTexture(t);
        }
        state = 0;
    }
}

Texture* 
textureSystemGetDefaultTexture()
{
    if(!pState->defaultTexture)
        textureCreateDefaultTexture();
    return pState->defaultTexture;
}

static bool 
loadTexture(const char* name, Texture** t)
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
    u32 currentGeneration = (*t)->generation;
    (*t)->generation = INVALID_ID;

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

    renderCreateTexture(textureData->pixels, &tempTexture);

    Texture oldTexture = *(*t);
    *(*t) = tempTexture;
    (*t)->id = oldTexture.id;

    renderDestroyTexture(&oldTexture);
    
    // Assign the generation
    if(currentGeneration == INVALID_ID){
        (*t)->generation = 0;
    } else {
        (*t)->generation = currentGeneration + 1;
    }

    resourceSystemUnload(&txt);
    return true;

}

Texture* 
textureSystemGet(const char* name, bool autoRelease /*= false*/)
{
    if(stringEquals(name, DEFAULT_TEXTURE_NAME)){
        return pState->defaultTexture;
    }

    void* reference = nullptr;
    if(pState && hashtableGetValue(&pState->hashtable, name, &reference))
    {
        TextureReference* ref = static_cast<TextureReference*>(reference);
        if(ref->referenceCount == 0) {
            ref->autoRelease = autoRelease;
        }

        ref->referenceCount++;
        if(ref->handle == INVALID_ID)
        {
            u32 count = pState->config.maxTextureCount;
            Texture* t = 0;
            for(u32 i = 0; i < count; ++i) {
                if(pState->textures[i].id == INVALID_ID)
                {
                    ref->handle = i;
                    t = &pState->textures[i];
                    break;
                }
            }

            // Make sure an sopt is actually found
            if(!t || ref->handle == INVALID_ID) {
                PFATAL("textureSystemGet - Texture system cannot hold anymore textures.");
                return nullptr;
            }

            // Create a new texture
            if(!loadTexture(name, &t)){
                PERROR("textureSystemGet - Could not load texture '%s'.", name);
                return nullptr;
            }

            // Use the handle as the texture id.
            t->id = ref->handle;
        }
        else {
            PINFO("Texture '%s' already exists. Increasing reference count to %i.", name, ref->referenceCount);
        }

        // Update entry
        hashtableSetValue(&pState->hashtable, name, &ref);
        return &pState->textures[ref->handle];
    }

    PERROR("textureSystemGet - failed to get texture '%s'.", name);
    return nullptr;
    
/*
   // If texture found, return texture
   auto it = pState->map.find(name);
   if(pState && (it != pState->map.end())) {
       return &pState->textures[it->second.handle];
   }
   else // Create new texture and store it
   {
        TextureReference* ref = new TextureReference(); 
        ref->autoRelease = autoRelease;
        Texture* t = nullptr;
        for(u32 i = 0; i < pState->config.maxTextureCount; ++i)
        {
            if(pState->textures[i].id == INVALID_ID)
            {
                ref->handle = i;
                t = &pState->textures[i];
                break;
            }
        }

        // Make sure a texture slot was available
        if(!t || ref->handle == 0)
        {
            PERROR("textureSystemGet - no slot available for a new texture. Increase the number of available texture slots.");
            return 0;
        }

        if(!loadTexture(name, t))
        {
            PERROR("textureSystemGet - could not load texture '%s'.", name);
            return 0;
        }

        t->id = ref->handle;
        pState->map[name] = *ref;
        return &pState->textures[ref->handle];

   }
*/
}

void 
textureSystemRelease(const char* name)
{
    if(stringEquals(name, DEFAULT_TEXTURE_NAME)){
        return;
    }

    TextureReference* ref = 0;
    void* ptr = static_cast<void*>(ref);
    if(pState && hashtableGetValue(&pState->hashtable, name, &ptr))
    {
        if(ref->referenceCount == 0) {
            PWARN("Tried to release a non-existent texture: '%s'.", name);
            return;
        }

        char nameCopy[TEXTURE_NAME_MAX_LENGTH];
        stringCopy(name, nameCopy);

        ref->referenceCount--;
        if(ref->referenceCount == 0 && ref->autoRelease)
        {
            Texture* t = &pState->textures[ref->handle];

            // Destroy texture, including in render side.
            textureSystemDestroyTexture(t);

            ref->handle = INVALID_ID;
            ref->autoRelease = false;
            PINFO("Released texture '%s'.", nameCopy);
        }
        else {
            PINFO("Released texture '%s'. It has now a reference count of %i.", nameCopy, ref->referenceCount);
        }

        hashtableSetValue(&pState->hashtable, nameCopy, &ref);
    }
    else {
        PERROR("textureSystemRelease - failed to release texture '%s'.", name);
    }

/*
    auto it = pState->map.find(name);
    if(pState && (it != pState->map.end()))
    {
        TextureReference* ref = &it->second;
        if(ref->referenceCount == 0)
        {
            PWARN("Trying to release non-existant texture '%s'.", name);
            return;
        }

        // Save a copy of the name, since it being a pointer may be erased when removing the texture.
        char nameCopy[TEXTURE_NAME_MAX_LENGTH];
        stringCopy(name, nameCopy);

        ref->referenceCount--;
        if(ref->referenceCount == 0 && ref->autoRelease)
        {
            Texture* t = &pState->textures[ref->handle];

            textureSystemDestroyTexture(t);

            ref->handle = INVALID_ID;
            ref->autoRelease = false;
            PINFO("Released texture '%s'.", nameCopy);
            pState->map[nameCopy] = *ref;
        }
    }
    PERROR("textureSystemRelease - Failed to release texture '%s'.", name);
    */
}
