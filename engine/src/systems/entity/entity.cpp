#include "systems/entity/entity.h"
#include "systems/components/comp_name.h"

DECL_OBJ_MANAGER("entity", CEntity);

CEntity::~CEntity() {
    for(u32 i = 1; i < CHandleManager::getNumDefinedTypes(); ++i) {
        CHandle h = comps[i];
        if(comps[i].isValid())
            comps[i].destroy();
    }
}

void CEntity::set(u32 type, CHandle newComp) {
    PASSERT(type < CHandle::maxTypes)
    PASSERT(!comps[type].isValid())
    comps[type] = newComp;
    newComp.setOwner(CHandle(this));
}

void CEntity::set(CHandle newComp) {
    set(newComp.getType(), newComp);
}

const char* CEntity::getName() const {
    TCompName* n = get<TCompName>();
    if(n)
        return n->getName();
    return "<Unnamed>";
}

void CEntity::onEntityCreated() {
    for(u32 i = 0; i < CHandleManager::getNumDefinedTypes(); ++i) {
        CHandle h = comps[i];
        h.onEntityCreated();
    }
}

void CEntity::renderDebug() {
    for(u32 i = 0; i < CHandleManager::getNumDefinedTypes(); ++i) {
        CHandle h = comps[i];
        if(h.isValid())
            h.renderDebug();
    }
}

void CEntity::debugInMenu() 
{
    ImGui::PushID(this);
    if(ImGui::TreeNode(getName()))
    {
        for(int i = 0; i < CHandle::maxTypes; ++i)
        {
            CHandle h = comps[i];
            if(h.isValid())
            {
                if(ImGui::TreeNode(h.getTypeName()))
                {
                    h.debugInMenu();
                    ImGui::TreePop();
                }
            }
        }
        ImGui::TreePop();
    }

    // TODO if hovered, render menu

    ImGui::PopID();
}

void CEntity::load(const json& j, TEntityParseContext& ctx)
{
    for (const auto& it : j.items()) {
        auto& compName = it.key();
        auto& compValue = it.value();

        PDEBUG("Parsing component '%s' from json '%s'.", compName.c_str(), j.dump().c_str());

        auto om = CHandleManager::getByName(compName.c_str());
        if(!om) {
            PWARN("Uknown component with name '%s'.", compName.c_str());
            continue;
        }

        u32 compType = om->getType();

        CHandle component = comps[compType];
        if(component.isValid())
        {
            // Reconfigure the component from the json
            component.load(compValue, ctx);
        }
        else
        {
            // Create a new fresh component and attach it to the entity
            component = om->createHandle();
            set(compType, component);
            component.load(compValue, ctx);
        }
    }
}