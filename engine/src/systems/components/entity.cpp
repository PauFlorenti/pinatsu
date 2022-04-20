#include "systems/entity/entity.h"

CEntity::~CEntity() {
    for(u32 i = 1; i < CHandleManager::getNumDefinedTypes(); ++i) {
        CHandle h = comps[i];
        if(comps[i].isValid())
            comps[i].destroy();
    }
}

void CEntity::set(u32 type, CHandle newComp) {
    PASSERT(type < CHandle::maxTypes)
    PASSERT(!comps[type].isValid())
    comps[type] = newComp;
    newComp.setOwner(CHandle(this));
}

void CEntity::set(CHandle newComp) {
    set(newComp.getType(), newComp);
}

const char* CEntity::getName() const {
    return "<Unnamed>"; // TODO use TCompName when available
}

void CEntity::onEntityCreated() {
    for(u32 i = 0; i < CHandleManager::getNumDefinedTypes(); ++i) {
        CHandle h = comps[i];
        h.onEntityCreated();
    }
}

void CEntity::renderDebug() {
    for(u32 i = 0; i < CHandleManager::getNumDefinedTypes(); ++i) {
        CHandle h = comps[i];
        if(h.isValid())
            h.renderDebug();
    }
}

void CEntity::debugInMenu() {
    // TODO ImGui shit
}

void CEntity::load(const json& j, TEntityParseContext& ctx)
{
    for (const auto& it : j.items()) {
        auto& compName = it.key();
        auto& compValue = it.value();
    }
}