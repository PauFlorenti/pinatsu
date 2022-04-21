#pragma once

struct TEntityParseContext;

struct TCompBase {
    void debugInMenu() {};
    void renderDebug() {};
    void load(const json& j, TEntityParseContext& ctx) {};
    void update(f32 dt) {};
    void onEntityCreated() {};
};

#define DECL_SIBILING_ACCESS()  \
    template<typename TComp>    \
    CHandle get() {             \
        CEntity* e = CHandle(this).getOwner();  \
        if(!e)                                  \
            return CHandle();                   \
        return e->get<TComp>();                 \
    }                                           \
    CEntity* getEntity() {                      \
        CEntity* e = CHandle(this).getOwner();  \
        return e;                               \
    }                                           \