#include "comp_base.h"

#include <external/glm/glm.hpp>
#include <external/glm/gtc/quaternion.hpp>
#include <external/glm/gtx/matrix_decompose.hpp>

struct TCompTransform : public TCompBase
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    void debugInMenu();
    void renderDebug();

    glm::mat4 asMatrix() {
        return glm::translate(glm::mat4(1), position)
                * glm::mat4_cast(rotation)
                * glm::scale(glm::mat4(1), scale);
    }

    void fromMatrix(glm::mat4 matrix) {
        glm::decompose(matrix, scale, rotation, position, glm::vec3(0), glm::vec4(0));
        rotation = glm::conjugate(rotation);
    }
};