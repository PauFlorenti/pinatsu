#pragma once
#include "defines.h"
#include "module.h"

class CModuleEntities : public IModule
{
    std::vector<CHandleManager*> toUpdate;

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