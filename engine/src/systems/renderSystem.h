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

    /** Necessary information to pass to the Renderer to draw. 
     * This is the DrawCall definition.*/
    struct TKey {
        Mesh* mesh = nullptr;
        Material* material = nullptr;
        CHandle hOwner;
        CHandle hTransform;
    };

    /** All active DrawCalls.*/ 
    std::vector<TKey> keys;
protected:
    bool keysAreDirty = false;

    /** Function to update DrawCalls each frame before rendering.
     * Updates transforms and should filter given some parameters such as transparency.*/
    void sortKeys();

public:
    /** Add a DrawCall if active to render it.*/
    void addKey(
        CHandle owner,
        Mesh* mesh,
        Material* material
    );

    /** Delete all DrawCalls from specific handler/component. */
    void deleteKeysFromOwner(CHandle hOwner);

    /** Render all submitted draw calls. */
    void render();

    void setActiveCamera(CHandle hCamera) { activeCamera = hCamera; }
    CHandle getActiveCamera() const { return activeCamera; }

    void setDirty() { keysAreDirty = true; }

private:
    CHandle activeCamera;
};