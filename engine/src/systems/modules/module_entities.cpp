#include "module_entities.h"
#include "systems/entity/entity.h"

void CModuleEntities::loadManagers(const json& j, std::vector<CHandleManager*>& managers)
{
    managers.clear();
    std::vector<std::string> names = j;
    for(auto& n : names)
    {
        auto om = CHandleManager::getByName(n.c_str());
        PASSERT_MSG(om, "Can't find a manager of type (SHOULD ADD PARAMETERS TO PASSERT_MSG) to update. Check file components.json.")
        managers.push_back(om);
    }
}

bool CModuleEntities::start()
{
    json j = loadJson("data/components/components.json");

    std::map<std::string, i32> componentSizes = j["sizes"];
    i32 defaultSize = componentSizes["default"];

    // Reorder the init manager based on the json
    std::map<std::string, i32> initOrder = j["init_order"];
    std::sort(CHandleManager::predefinedManagers, 
        CHandleManager::predefinedManagers + CHandleManager::nPredefinedManagers,
        [&initOrder](CHandleManager* m1, CHandleManager* m2){
            i32 priority1 = initOrder[m1->getName()];
            i32 priority2 = initOrder[m2->getName()];
            return priority1 > priority2;
        });

    // Important that entity is the first one for the chain destruction of components.
    PASSERT(strcmp(CHandleManager::predefinedManagers[0]->getName(), "entity") == 0)

    for(size_t i = 0; i < CHandleManager::nPredefinedManagers; ++i)
    {
        auto om = CHandleManager::predefinedManagers[i];
        auto it = componentSizes.find(om->getName());
        i32 size = (it == componentSizes.end()) ? defaultSize : it->second;
        PDEBUG("Initializing obj manager '%s' with a size of %d.", om->getName(), size);
        om->init(size);
    }

    loadManagers(j["update"], toUpdate);
    //TODO to render debug? loadManagers(j["render"], )

    return true;
}

void CModuleEntities::stop()
{
    CHandleManager::destroyAllPendingObjects();
}

void CModuleEntities::update(f32 dt)
{
    //PINFO("Updating module entities ...")
    for(auto om : toUpdate)
    {
        if(om)
            om->updateAll(dt);
        CHandleManager::destroyAllPendingObjects();
    }
}

void CModuleEntities::renderInMenu() {
    // TODO imgui rendering here ...
    for(auto om : toUpdate)
    {
        if(om)
            om->debugInMenuAll();
    }
}

void CModuleEntities::renderDebug() {
    // TODO render debug in here ...
}

void CModuleEntities::render() {

}