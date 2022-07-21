#include "module_manager.h"

void CModuleManager::boot()
{
    parseModulesConfig("data/modules.json");
}

void CModuleManager::clear(){}

void CModuleManager::update(f32 dt)
{
    for(auto module : updatedModules)
    {
        if(module->isActive())
        {
            module->update(dt);
        }
    }
}

void CModuleManager::render()
{
    for(auto module : renderedModules)
    {
        if(module->isActive())
        {
            module->render();
        }
    }
}

void CModuleManager::renderInMenu()
{
    for(auto module : registeredModules)
    {
        module.second->renderInMenu();
    }
}

void CModuleManager::renderDebug()
{
    for(auto module : registeredModules)
    {
        module.second->renderDebug();
    }
}

IModule* CModuleManager::getModule(const std::string& name)
{
    auto moduleIt = registeredModules.find(name);
    return moduleIt != registeredModules.end() ? moduleIt->second : nullptr;
}

void CModuleManager::registerServiceModule(IModule* module)
{
    registeredModules[module->getName()] = module;

    if(module->doStart())
        services.push_back(module);
}

void CModuleManager::registerGameModule(IModule* module)
{
    registeredModules[module->getName()] = module;
}

void CModuleManager::parseModulesConfig(const std::string& filename)
{
    updatedModules.clear();
    renderedModules.clear();

    json jData = loadJson(filename);

    json jUpdatedList = jData["update"];
    json jRenderedList = jData["render"];

    if(jUpdatedList.is_null()){
        PERROR("No modules to update.")
    }

    if(jRenderedList.is_null()){
        PERROR("No modules to render.")
    }

    for(auto jModule : jUpdatedList)
    {
        const std::string& moduleName = jModule.get<std::string>();
        IModule* module = getModule(moduleName);
        PASSERT(module != nullptr);
        if(module)
        {
            updatedModules.push_back(module);
        }
    }

    for(auto jModule : jRenderedList)
    {
        const std::string& moduleName = jModule.get<std::string>();
        IModule* module = getModule(moduleName);
        PASSERT(module != nullptr);
        if(module)
        {
            renderedModules.push_back(module);
        }
    }
}

void CModuleManager::startModules(std::vector<IModule*> modules)
{
    for(auto module : modules)
        module->doStart();
}

void CModuleManager::stopModules(std::vector<IModule*> modules)
{
    for(auto module : modules)
        module->doStart();
}