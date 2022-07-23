#pragma once

/**
 * Helper transform class.
 */

class CTransform
{
    glm::vec3 position;
    glm::vec3 scale = glm::vec3(1.0f);
    glm::quat rotation;

public:
    void setRotation(glm::quat newRotation) { rotation = newRotation; }
    void setPosition(glm::vec3 newPos) { position = newPos; }
    void setScale(glm::vec3 newScale) { scale = newScale; }

    glm::quat getRotation() const { return rotation; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getScale() const { return scale; }

    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

    glm::mat4 asMatrix() const;
    void fromMatrix(glm::mat4 matrix);
    CTransform combinedWith(const CTransform& delta_transform) const;

    void setEulerAngles(f32 yaw, f32 pitch, f32 roll);
    void getEulerAngles(f32* yaw, f32* pitch, f32* roll) const;

    void lookAt(glm::vec3 eye, glm::vec3 target, glm::vec3 up);
    bool fromJson(const json& j);

    bool renderInMenu();
    bool renderGuizmo();
};