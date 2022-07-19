#include "renderSystem.h"

#include "resourceSystem.h"

CRenderManager* CRenderManager::instance = nullptr;

void CRenderManager::addKey(CHandle owner, Mesh* mesh, Material* material)
{
    PASSERT(mesh);
    PASSERT(material);

    TKey key;
    key.material = material;
    key.mesh = mesh;
    key.hOwner = owner;

    keys.push_back(key);
}

void CRenderManager::deleteKeysFromOwner(CHandle hOwner)
{
    auto it = std::remove_if(
        keys.begin(), 
        keys.end(), 
        [hOwner](const TKey& k1){return k1.hOwner == hOwner;});
    
    keys.erase(it);
}

void CRenderManager::render()
{
    // TODO Sort keys if dirty.

    u32 nDrawCalls = 0;

    TKey nullKey = {};
    const TKey* previousKey = &nullKey;
    const TKey* key = &nullKey;

    auto it = keys.begin();
    auto end = keys.end();

/*     while(it != end)
    {
        key = &(*it);

        
    } */
}