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