#pragma once
#include "handleManager.h"
#include "defines.h"

struct TEntityParseContext;

template<class TObj>
class CObjectManager : public CHandleManager
{
    std::vector<u8> allocatedMemory;

    TObj* objs = nullptr;

    void createObj(u32 internalIndex) override {
        TObj* address = objs + internalIndex;
        new (address) TObj;
    }

    void destroyObj(u32 internalIndex) override {
        TObj* address = objs + internalIndex;
        address->~TObj();
    }

    void moveObj(u32 srcInternalIndex, u32 dstInternalIndex) override {
        TObj* src = objs + srcInternalIndex;
        TObj* dst = objs + dstInternalIndex;
        new(dst)TObj(std::move(*src));
    }

    void debugInMenuObj(u32 internalIndex) override {
        TObj* address = objs + internalIndex;
        address->debugInMenu();
    }

    void renderDebugObj(u32 internalIndex) override {
        TObj* address = objs + internalIndex;
        address->renderDebug();
    }

    void onEntityCreatedObj(u32 internalIndex) override {
        TObj* address = objs + internalIndex;
        address->onEntityCreated();
    }

    void loadObj(u32 internalIndex, const json& j, TEntityParseContext& ctx) override {
        TObj* address = objs + internalIndex;
        address->load(j, ctx);
    }

public:
    CObjectManager(const CObjectManager&) = delete;
    CObjectManager(const char* newName) : objs(nullptr) {
        name = newName;
        CHandleManager::predefinedManagers[CHandleManager::nPredefinedManagers] = this;
        CHandleManager::nPredefinedManagers++;
    }

    void init(u32 maxObjects) override {
        CHandleManager::init(maxObjects);

        allocatedMemory.resize(maxObjects * sizeof(TObj));

        objs = static_cast<TObj*>((void*)allocatedMemory.data());
    }

    CHandle getHandleFromAddress(TObj* address) {
        auto internalIndex = address - objs;
        if(internalIndex >= nObjectsUsed || internalIndex < 0)
            return CHandle();
        auto externalIndex = internalToExternal[internalIndex];
        auto& ed = externalToInternal[externalIndex];
        return CHandle(type, externalIndex, ed.currentAge);
    }

    TObj* getAddressFromHandle(CHandle h) {
        if(!h.getType())
            return nullptr;
        
        if(h.getType() != getType()){
            PFATAL("You have requested to convert a handle of type %s to a class of type %s.",
                CHandleManager::getType(h.getType())->getName(), getName())
            return nullptr;
        }
        PASSERT(h.getType() == getType())
        const auto& ed = externalToInternal[h.getIndex()];
        if(ed.currentAge != h.getAge())
            return nullptr;
        
        return objs + ed.internalIndex;
    }

    void updateAll(f32 dt) override {
        PASSERT(objs);

        if(!nObjectsUsed)
            return;

        for(u32 i = 0; i < nObjectsUsed; ++i) {
            objs[i].update(dt);
        }
    }

    void renderDebugAll() override {
        PASSERT(objs)
        for(u32 i = 0; i < nObjectsUsed; ++i) {
            objs[i].renderDebug();
        }
    }

    template<typename TFn>
    void forEach(TFn fn) {
        PASSERT(objs)
        for(u32 i = 0; i < nObjectsUsed; ++i) {
            fn(bojs + i);
        }
    }

    template<typename TFn>
    void forEachWithExternalIndex(TFn fn) {
        PASSERT(objs)
        for(u32 i = 0; i < nObjectsUsed; ++i) {
            fn(objs + i, internalToExternal[i]);
        }
    }

    void debugInMenuAll() {
        PASSERT(objs)

        //char buf[80];
        // Report usage information about the object type ...
    }

};

#include <stdio.h>

#define DECL_OBJ_MANAGER(objName, objClassName) \
    CObjectManager<objClassName> om_ ## objClassName(objName); \
    template<> \
    CObjectManager<objClassName>* getObjectManager<objClassName>() { \
        return &om_ ## objClassName; \
    }
