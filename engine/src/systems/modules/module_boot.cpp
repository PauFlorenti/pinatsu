#include "module_boot.h"

bool CModuleBoot::start()
{
    json j = loadJson("data/boot.json");
    auto scenes = j["scenes_to_load"];
    for(auto scene : scenes)
    {
        loadScene(scene);
    }
    return true;
}

void CModuleBoot::stop() {};

void CModuleBoot::loadScene(const std::string& sceneName)
{
    TEntityParseContext ctx;
    PDEBUG("Parsing scene %s.", sceneName.c_str());
    parseScene(sceneName, ctx);
    ctxs.push_back(ctx);
}