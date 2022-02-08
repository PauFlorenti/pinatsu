#include "materialSystem.h"

#include "core/logger.h"
#include "core/pstring.h"
#include "memory/pmemory.h"
#include "renderer/rendererFrontend.h"

#include "systems/textureSystem.h"

typedef struct MaterialSystemState
{
    Material* defaultMaterial;
    MaterialSystemConfig config;
    Material* materials;
} MaterialSystemState;

static MaterialSystemState* pState;

void materialSystemCreateDefaultMaterial()
{
    pState->defaultMaterial = (Material*)memAllocate(sizeof(Material), MEMORY_TAG_MATERIAL_INSTANCE);
    pState->defaultMaterial->diffuseColor = glm::vec4(1, 0, 0, 1);
    stringCopy("Default", pState->defaultMaterial->name);
    pState->defaultMaterial->id         = 0;
    pState->defaultMaterial->rendererId = INVALID_ID;
    pState->defaultMaterial->generation = 0;

    renderCreateMaterial(pState->defaultMaterial);
}

bool materialSystemInit(u64* memoryRequirements, void* state, MaterialSystemConfig config)
{
    *memoryRequirements = sizeof(MaterialSystemConfig) + sizeof(Material) * config.maxMaterialCount;
    if(!state){
        return true;
    }

    pState = (MaterialSystemState*)state;
    pState->config = config;
    pState->materials = (Material*)(pState + sizeof(MaterialSystemConfig));

    for(u32 i = 0; i < pState->config.maxMaterialCount; ++i) {
        pState->materials[i].id         = INVALID_ID;
        pState->materials[i].rendererId = INVALID_ID;
        pState->materials[i].generation = INVALID_ID;
    }

    //materialSystemCreateDefaultMaterial();
    //pState->materials[0] = *pState->defaultMaterial;
    return true;
}

void materialSystemShutdown(void* state)
{
    if(pState){

        for(u32 i = 0; i < pState->config.maxMaterialCount; ++i)
        {
            // TODO destroy material
            pState->materials[i].id = INVALID_ID;
        }
        pState = nullptr;
    }
}

Material* materialSystemGetMaterialByName(const char* name)
{
    if(stringEquals(name, "Default")){
        return pState->defaultMaterial;
    }

    // TODO
    return nullptr;
}

Material* materialSystemCreateFromData(MaterialData data)
{
    // TODO select from hastable
    // Select first empty material
    Material* mat = nullptr;
    for(u32 i = 0; i < pState->config.maxMaterialCount; ++i)
    {
        if(pState->materials[i].id == INVALID_ID){
            mat = &pState->materials[i];
            mat->id = i;
            break;
        }
    }

    mat->type = data.type;
    mat->diffuseColor = data.diffuseColor;
    mat->diffuseTexture = stringLength(data.diffuseTextureName) > 0 ? textureSystemGet(data.diffuseTextureName) : nullptr;
    if(mat->diffuseTexture) mat->diffuseTexture->use = TEXTURE_USE_DIFFUSE;
    mat->normalTexture = stringLength(data.normalTextureName) > 0 ? textureSystemGet(data.normalTextureName) : nullptr;
    if(mat->normalTexture) mat->normalTexture->use = TEXTURE_USE_NORMAL;
    mat->metallicRoughnessTexture = stringLength(data.metallicRoughnessTextureName) > 0 ? textureSystemGet(data.metallicRoughnessTextureName) : nullptr;
    if(mat->metallicRoughnessTexture) mat->metallicRoughnessTexture->use = TEXTURE_USE_METALLIC_ROUGHNESS;
    // TODO copy name

    if(!renderCreateMaterial(mat)){
        PERROR("materialSystemCreateFromData - Could not create material in render side.");
        return nullptr;
    }

    if(mat->generation == INVALID_ID)
    {
        mat->generation = 0;
    } else {
        mat->generation++;
    }

    return mat;
}