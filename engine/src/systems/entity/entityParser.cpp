#include "entityParser.h"

TEntityParseContext::TEntityParseContext(TEntityParseContext& another, const TCompTransform& deltaTransform)
{
    parent = &another;
    recursionLevel = another.recursionLevel + 1;
    rootEntity = another.rootEntity;
    rootTransform = another.rootTransform.combinedWith(deltaTransform);
}

CHandle spawn(const std::string& filename, TCompTransform root)
{
    TEntityParseContext ctx;
    ctx.rootTransform = root;
    if(!parseScene(filename, ctx))
        return CHandle();
    PASSERT(!ctx.entitiesLoaded.empty());
    return ctx.entitiesLoaded[0];
}

bool parseScene(const std::string& filename, TEntityParseContext& ctx)
{
    ctx.filename = filename;

    return true;
    
}