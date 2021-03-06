#include "entityParser.h"
#include "systems/entity/entity.h"

TEntityParseContext::TEntityParseContext(TEntityParseContext& another, const CTransform& deltaTransform)
{
    parent = &another;
    recursionLevel = another.recursionLevel + 1;
    rootEntity = another.rootEntity;
    rootTransform = another.rootTransform.combinedWith(deltaTransform);
}

CHandle spawn(const std::string& filename, CTransform root)
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

    const json& j = loadJson("data/scenes/" + filename);

    for(u32 i = 0; i < j.size(); ++i)
    {
        const json& jitem = j[i];
        PASSERT(jitem.is_object());

        if(jitem.count("entity")) {
            const json& jentity = jitem["entity"];

            CHandle hentity;
            
            // TODO Prefabs
            hentity.create<CEntity>();
            CEntity* entity = hentity;
            entity->load(jentity, ctx);
            ctx.allEntitiesLoaded.push_back(hentity);
            ctx.entitiesLoaded.push_back(hentity);
        }

        // TODO if multiple entities, do hierarchy
        if(ctx.entitiesLoaded.size() > 1)
        {
            
        }

        if(!ctx.parsingPrefab) {
            for(auto h : ctx.allEntitiesLoaded)
                h.onEntityCreated();
        }
    }
    return true;
}