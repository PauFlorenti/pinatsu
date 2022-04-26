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