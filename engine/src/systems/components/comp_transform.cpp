#include "comp_transform.h"

#include "comp_name.h"
#include "comp_camera.h"
#include "systems/entity/entityParser.h"
#include "systems/entity/entity.h"

 DECL_OBJ_MANAGER("transform", TCompTransform);

void
TCompTransform::load(const json& j, TEntityParseContext& ctx)
{
    CTransform::fromJson(j);
    //set(ctx.rootTransform.combinedWith(*this));
}

void TCompTransform::set(const CTransform& newT)
{
    *(CTransform*)this = newT;
}

void 
TCompTransform::debugInMenu()
{
    CTransform::renderInMenu();
}

void TCompTransform::renderDebug()
{

}