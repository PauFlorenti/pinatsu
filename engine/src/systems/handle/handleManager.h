#pragma once

#include "handle.h"

#include <vector>
#include <map>
#include <string>
#include "core/logger.h"
#include "core/assert.h"

class CHandleManager {

    static const u32 maxTotalObjectsAllowed = 1 << CHandle::nBitsIndex;
    static const u32 invalidIndex = ~0;
    static bool anyHandleDestroyed;

    struct ExternalData {
        u32 internalIndex;
        u32 nextExternalIndex;
        CHandle currentOwner;
        u32 currentAge : CHandle::nBitsAge;

        ExternalData()
            : internalIndex(0)
            , currentAge(0)
            , nextExternalIndex(0) {}
    };

protected:

    u32 type;
    std::vector<ExternalData> externalToInternal;
    std::vector<u32> internalToExternal;
    u32 nObjectsUsed;
    u32 nextFreeHandleExternalIndex;
    u32 lastFreeHandleExternalIndex;

    // Handles to be destroyed in a safe moment.
    std::vector<CHandle> objToDestroy;
    const char* name = nullptr;

    // Shared by all managers.
    static u32                                      nextTypeOfHandleManager;
    // Handle array of type manager to easily access any manager.
    static CHandleManager*                          allManagers[CHandle::maxTypes];
    // Map to easily access any manager by name.
    static std::map<std::string, CHandleManager*>   allManagersByName;

// -----------------------------------------------------------------
    virtual void createObj(u32 internal_idx) = 0;
    virtual void destroyObj(u32 internal_idx) = 0;
    virtual void moveObj(u32 src_internal_idx, u32 dst_internal_idx) = 0;
    // virtual void loadObj(u32 src_internal_idx, const json& j, TEntityParseContext& ctx) = 0;
    virtual void debugInMenuObj(u32 internal_idx) = 0;
    virtual void renderDebugObj(u32 internal_idx) = 0;
    virtual void onEntityCreatedObj(u32 internal_idx) = 0;

public:
    virtual void init(u32 maxObjects) {
        PASSERT(maxObjects < maxTotalObjectsAllowed);
        PASSERT(maxObjects > 0);

        if (type == 0) {
            type = nextTypeOfHandleManager;
            nextTypeOfHandleManager++;
        }

        allManagers[type] = this;
        allManagersByName[getName()] = this;

        nObjectsUsed = 0;
        u32 nObjectsCapacity = maxObjects;

        externalToInternal.resize(nObjectsCapacity);
        internalToExternal.resize(nObjectsCapacity);

        u32 i = 0;
        for (auto& ed : externalToInternal){
            ed.currentAge = 1;
            ed.internalIndex = invalidIndex;
            if(i != nObjectsCapacity - 1)
                ed.nextExternalIndex = i + 1;
            else 
                ed.nextExternalIndex = invalidIndex;
            internalToExternal[i] = invalidIndex;
            ++i;
        }

        nextFreeHandleExternalIndex = 0;
        lastFreeHandleExternalIndex = nObjectsCapacity - 1;
    }

    bool isValid(CHandle h) const {
        PASSERT(h.getType() == type);
        PASSERT(h.getExternalIndex() < capacity());
        auto& ed = externalToInternal[h.getExternalIndex()];
        return ed.currentAge == h.getAge();
    }

    const char* getName() const { return name; }
    u32 getType() const { return type; }
    u32 size() const { return nObjectsUsed; }
    u32 capacity() const { return (u32)externalToInternal.size(); }

    bool destroyPendingObjects();

    CHandle createHandle();
    void destroyHandle(CHandle h);
    void debugInMenu(CHandle h);
    void renderDebug(CHandle h);
    void onEntityCreated(CHandle h);
    void load(CHandle h); // TODO make this able to load from json

    // Methods applying to all objects
    virtual void updateAll(f32 dt) = 0;
    virtual void renderDebugAll() = 0;
    virtual void debugInMenuAll() = 0;

    void setOwner(CHandle who, CHandle newOwner);
    CHandle getOwner(CHandle who);

    static CHandleManager* getByType(u32 type);
    static CHandleManager* getByName(const char* name);
    static u32 getNumDefinedTypes();
    static void destroyAllPendingObjects();
    void dumpInternals() const;
}