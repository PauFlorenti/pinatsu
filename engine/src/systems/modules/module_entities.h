#pragma once
#include "module.h"

class CModuleEntities : public IModule
{
    std::vector<CHandleManager*> toUpdate;
    std::vector<CHandleManager*> toRenderDebug;

    void loadManagers(const json& j, std::vector<CHandleManager*>& managers);
    void renderDebugOfComponents();
    void editRenderDebug();

public:
    CModuleEntities(const std::string name) : IModule(name) {}
    bool start() override;
    void stop() override;
    void update(f32 dt) override;
    void render() override;
    void renderDebug() override;
    void renderInMenu() override;
};