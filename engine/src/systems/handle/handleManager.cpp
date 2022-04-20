#include "handleManager.h"

// Zero is predefined as invalid component
u32                                     CHandleManager::nextTypeOfHandleManager = 1;
CHandleManager*                         CHandleManager::allManagers[CHandle::maxTypes];
std::map<std::string, CHandleManager*>  CHandleManager::allManagersByName;

CHandleManager* CHandleManager::predefinedManagers[CHandle::maxTypes];
u32             CHandleManager::nPredefinedManagers = 0;
bool            CHandleManager::anyHandleDestroyed = false;

void CHandleManager::destroyAllPendingObjects() {
    if(!anyHandleDestroyed)
        return;

    bool somethingDeleted = false;
    do {
        somethingDeleted = false;
        for(u32 i = 1; i < getNumDefinedTypes(); ++i) {
            somethingDeleted |= allManagers[i]->destroyPendingObjects();
        }
    } while (somethingDeleted);
    anyHandleDestroyed = false;
}

u32 CHandleManager::getNumDefinedTypes() {
    return nextTypeOfHandleManager;
}

CHandleManager* CHandleManager::getByType(u32 type) {
    return allManagers[type];
}

CHandleManager* CHandleManager::getByName(const char* name) {
    auto it = allManagersByName.find(name);
    if(it == allManagersByName.end())
        return nullptr;
    return it->second;
}

CHandle CHandleManager::createHandle() {

    PASSERT(type != 0)

    const u32 nObjectsCapacity = capacity();

    PASSERT(nextFreeHandleExternalIndex != invalidIndex)
    PASSERT(nObjectsUsed < nObjectsCapacity)
    u32 externalIndex = nextFreeHandleExternalIndex;
    auto& ed = externalToInternal[externalIndex];

    ed.internalIndex = nObjectsUsed;

    // Owner should have been deleted ...
    PASSERT(ed.currentOwner == CHandle())

    internalToExternal[ed.internalIndex] = externalIndex;

    createObj(ed.internalIndex);

    ++nObjectsUsed;

    // Update where is the next free for the next time we create another obj.
    nextFreeHandleExternalIndex = ed.nextExternalIndex;
    PASSERT_MSG(nextFreeHandleExternalIndex != invalidIndex, "We run out of objects.")

    ed.nextExternalIndex = invalidIndex;

    return CHandle(type, externalIndex, ed.currentAge);
}

void CHandleManager::destroyHandle(CHandle h)
{
    if(!isValid(h))
        return;

    anyHandleDestroyed = true;

    // Set to vector for objects pending to be destroyed.
    objToDestroy.push_back(h);

    // Update current age to invalidate handle.
    auto externalIndex = h.getIndex();
    auto& ed = externalToInternal[externalIndex];
    ed.currentAge++;
}

bool CHandleManager::destroyPendingObjects() 
{
    for(auto h : objToDestroy)
    {
        auto externalIndex = h.getIndex();
        auto& ed = externalToInternal[externalIndex];

        auto internalIndex = ed.internalIndex;

        PASSERT(nObjectsUsed > 0)

        ed.currentAge--;

        destroyObj(internalIndex);

        ed.currentAge++;

        PASSERT(lastFreeHandleExternalIndex != invalidIndex)
        auto& lastFreeEd = externalToInternal[lastFreeHandleExternalIndex];
        PASSERT(lastFreeEd.nextExternalIndex == invalidIndex)

        lastFreeHandleExternalIndex = externalIndex;

        PASSERT(ed.nextExternalIndex == invalidIndex);

        u32 internalIndexOfLastValidObject = nObjectsUsed - 1;
        if(internalIndex < internalIndexOfLastValidObject) {
            moveObj(internalIndexOfLastValidObject, internalIndex);

            auto movedObjectExternalIndex = internalToExternal[internalIndexOfLastValidObject];
            internalToExternal[internalIndex] = movedObjectExternalIndex;

            auto& movedObjectEd = externalToInternal[movedObjectExternalIndex];
            movedObjectEd.internalIndex = internalIndex;
        }

        nObjectsUsed--;
    }

    bool somethingDeleted = !objToDestroy.empty();

    objToDestroy.clear();

    return somethingDeleted;
}

void CHandleManager::setOwner(CHandle who, CHandle newOwner)
{
    PASSERT(who.isValid())
    auto& ed = externalToInternal[who.getIndex()];
    ed.currentOwner = newOwner;
}

CHandle CHandleManager::getOwner(CHandle who) {
    if(!who.isValid())
        return CHandle();
    auto& ed = externalToInternal[who.getIndex()];
    return ed.currentOwner;
}

void CHandleManager::debugInMenu(CHandle who) {
    if(!who.isValid())
        return;
    auto& ed = externalToInternal[who.getIndex()];
    debugInMenuObj(ed.internalIndex);
}

void CHandleManager::renderDebug(CHandle who) {
    if(!who.isValid())
        return;

    auto& ed = externalToInternal[who.getIndex()];
    renderDebugObj(ed.internalIndex);
}

void CHandleManager::onEntityCreated(CHandle who) {
    if(!who.isValid())
        return;
    auto& ed = externalToInternal[who.getIndex()];
    onEntityCreatedObj(ed.internalIndex);
}

void CHandleManager::load(CHandle who, const json& j, TEntityParseContext& ctx) {
    if(!who.isValid())
        return;
    auto& ed = externalToInternal[who.getIndex()];
    loadObj(ed.internalIndex, j, ctx);
}

void CHandleManager::dumpInternals() const {
    // TODO understand this functionality.
}