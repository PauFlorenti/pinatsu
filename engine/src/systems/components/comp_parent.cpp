#include "comp_parent.h"
#include "systems/entitySystemComponent.h"

void TCompParent::addChild(const Entity& parent, const Entity& ent){

    // Make sure child has no other parent, only one is valid.
    if(EntitySystem::Get()->hasComponent<TCompParent>(ent)){
        TCompParent* childParentComponent = &EntitySystem::Get()->getComponent<TCompParent>(ent);
        if(childParentComponent->parent != 0){
            TCompParent* parentComponent = &EntitySystem::Get()->getComponent<TCompParent>(childParentComponent->parent);
            parentComponent->delChild(ent);
        }
        children.push_back(ent);
        childParentComponent->parent = parent;
        return;
    }
    
    TCompParent parentComponent{};
    parentComponent.parent = parent;
    EntitySystem::Get()->addComponent<TCompParent>(ent, parentComponent);
    children.push_back(ent);
}

void TCompParent::delChild(const Entity ent){

    i32 idx = 0;
    for(auto child : children){
        if(child == ent){
            children.erase(children.begin() + idx);
        }
        idx++;
    }
    TCompParent* parentComponent = &EntitySystem::Get()->getComponent<TCompParent>(ent);
    parentComponent->parent = 0;
}

void TCompParent::debugInMenu(){
    if(ImGui::TreeNode("Children ... "))
    {
        // TODO Rethink how to get all info from children ...
        ImGui::TreePop();
    }
}