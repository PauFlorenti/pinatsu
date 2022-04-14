#include "comp_base.h"
#include <vector>

using Entity = u32;

struct TCompParent : public TCompBase
{
    std::vector<Entity> children;
    Entity parent;
    void addChild(const Entity& parent, const Entity& ent);
    void delChild(const Entity ent);
    void debugInMenu();
};