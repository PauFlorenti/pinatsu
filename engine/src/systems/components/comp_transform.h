#pragma once
#include "comp_base.h"

struct TCompTransform : public TCompBase
{
    glm::vec3 position  = glm::vec3(1.0f);
    glm::quat rotation  = glm::quat();
    glm::vec3 scale     = glm::vec3(1.0f);

    void debugInMenu();
    void renderDebug();
    void load(const json& j, TEntityParseContext& ctx);

    glm::mat4 asMatrix() 
    {
        return glm::translate(glm::mat4(1), position)
                * glm::mat4_cast(rotation)
                * glm::scale(glm::mat4(1), scale);
    }

    void fromMatrix(glm::mat4 matrix) 
    {
        glm::decompose(
            matrix, 
            scale, 
            rotation, 
            position, 
            glm::vec3(0), 
            glm::vec4(0));
    }

    TCompTransform combinedWith(const TCompTransform& deltaTransform) const {
        TCompTransform newTransform;
        newTransform.rotation = deltaTransform.rotation * rotation;
        glm::vec3 deltaPosRotated = rotation * deltaTransform.position;
        newTransform.scale = scale * deltaTransform.scale;
        return newTransform;
    }
};