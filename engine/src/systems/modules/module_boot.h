#pragma once
#include "module.h"
#include "systems/entity/entityParser.h"

class CModuleBoot : public IModule
{
private:
    std::vector<TEntityParseContext> ctxs;
    void loadScene(const std::string& sceneName);
public:
    CModuleBoot(const std::string& name) : IModule(name) {};

    bool start() override;
    void stop() override;
};