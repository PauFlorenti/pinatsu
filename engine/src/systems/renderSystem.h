#pragma once

struct Mesh;
struct Material;

class CRenderManager
{
    static CRenderManager* instance;
public:

    /** Singleton getter.*/
    static CRenderManager* Get()
    {
        if(!instance){
            instance = new CRenderManager();
        }
        return instance;
    }

    /** Necessary information to pass to the Renderer to draw.*/
    struct TKey {
        Mesh* mesh = nullptr;
        Material* material = nullptr;
        CHandle hOwner;
        CHandle hTransform;
    };

    /** All active DrawCalls.*/ 
    std::vector<TKey> keys;

    /** Add a DrawCall if active to render it.*/
    void addKey(
        CHandle owner,
        Mesh* mesh,
        Material* material
    );

    /** Delete all DrawCalls from specific handler/component. */
    void deleteKeysFromOwner(CHandle hOwner);

    // TODO Sort keys function

    /** Render all submitted draw calls. */
    void render();
};