#pragma once

#include "module.h"

class CModuleManager
{
public:
    void boot();
    void clear();

    void update(f32 dt);
    void render();
    void renderInMenu();
    void renderDebug();

    void registerServiceModule(IModule* module);
    void registerGameModule(IModule* module);

    IModule* getModule(const std::string& name);

private:

    void parseModulesConfig(const std::string& filename);

    void startModules(std::vector<IModule*> modules);
    void stopModules(std::vector<IModule*> modules);

    std::vector<IModule*> services;
    std::vector<IModule*> updatedModules;
    std::vector<IModule*> renderedModules;

    std::map<std::string, IModule*> registeredModules;
};