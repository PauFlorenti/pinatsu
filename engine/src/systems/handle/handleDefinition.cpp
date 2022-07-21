#include "handle.h"

#include "handleManager.h"

bool CHandle::isValid() const {
    auto hm = CHandleManager::getByType(type);
    return hm && hm->isValid(*this);
}

void CHandle::destroy() {
    auto hm = CHandleManager::getByType(type);
    if(hm)
        hm->destroyHandle(*this);
}

const char* CHandle::getTypeName() const {
    auto hm = CHandleManager::getByType(type);
    if(hm)
        return hm->getName();
    return "<invalid>";
}

void CHandle::debugInMenu() {
    auto hm = CHandleManager::getByType(type);
    if(hm)
        hm->debugInMenu(*this);
}

void CHandle::renderDebug() {
    auto hm = CHandleManager::getByType(type);
    if(hm)
        hm->renderDebug(*this);
}

void CHandle::onEntityCreated() {
    auto hm = CHandleManager::getByType(type);
    if(hm)
        hm->onEntityCreated(*this);
}

void CHandle::load(const json& j, TEntityParseContext& ctx) {
    auto hm = CHandleManager::getByType(type);
    if(hm)
        hm->load(*this, j, ctx);
}

void CHandle::setOwner(CHandle newOwner) {
    auto hm = CHandleManager::getByType(type);
    if(hm)
        hm->setOwner(*this, newOwner);
}

CHandle CHandle::getOwner() {
    auto hm = CHandleManager::getByType(type);
    if(hm)
        return hm->getOwner(*this);
    return CHandle();
}