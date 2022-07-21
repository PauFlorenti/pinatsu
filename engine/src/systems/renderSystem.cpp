#include "renderSystem.h"

#include "resourceSystem.h"
#include "systems/components/comp_transform.h"
#include "systems/entity/entity.h"

CRenderManager* CRenderManager::instance = nullptr;

void CRenderManager::addKey(CHandle owner, Mesh* mesh, Material* material)
{
    PASSERT(mesh);
    PASSERT(material);
    PASSERT(owner.isValid())

    TKey key;
    key.material = material;
    key.mesh = mesh;
    key.hOwner = owner;
    key.hTransform = CHandle();

    keys.push_back(key);

    // New keys have been added, pending to sort them.
    keysAreDirty = true;
}

void CRenderManager::sortKeys()
{
    // TODO filter given some parameters

    for(auto& k : keys) {
        if(k.hTransform.isValid() == false)
        {
            CEntity* owner = k.hOwner.getOwner();
            PASSERT(owner)
            k.hTransform = owner->get<TCompTransform>();
        }
    }
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
    if(keysAreDirty)
        sortKeys();

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