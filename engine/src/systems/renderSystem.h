#pragma once

struct Mesh;
struct Material;

class CRenderManager
{
    static CRenderManager* instance;
public:

    static CRenderManager* Get()
    {
        if(!instance){
            instance = new CRenderManager();
        }
        return instance;
    }

    struct TKey {
        Mesh* mesh = nullptr;
        Material* material = nullptr;
        CHandle hOwner;
        CHandle hTransform;
    };

    std::vector<TKey> keys;

    void addKey(
        CHandle owner,
        Mesh* mesh,
        Material* material
    );
};