#include "comp_tag.h"
#include "systems/entity/entityParser.h"

DECL_OBJ_MANAGER("tag", TCompTag)

void TCompTag::load(const json& j, TEntityParseContext& ctx)
{
    CHandle(this).setOwner(ctx.currentEntity);

    for(u32 i = 0; i < maxTags; ++i)
        tags[i] = 0;

    PASSERT(j.is_array());
    u32 idx = 0;
    for(auto tag : j)
    {
        PASSERT(idx < maxTags);
        auto tagName = tag.get<std::string>();
        // u32 tagId = getID(tagName.c_str());

        // TODO some shit ...
    }
}