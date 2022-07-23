#pragma once

class CCamera
{
protected:
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 viewProjection;

    glm::vec3 eye;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 target;

    f32 aspectRatio = 1.0f;
    f32 fovDeg = 45.0f;
    f32 zmin = 0.1f;
    f32 zmax = 1000.0f;

    // options
    f32 speed;
    f32 zoom;

    bool isOrtho;

    void updateViewProjection();

public:
    // TODO should be managed by a camera component of the game
    bool locked;

    const glm::mat4& getView() const { return view; }
    const glm::mat4& getProjection() const { return projection; }
    const glm::mat4& getViewProjection() const { return viewProjection; }

    glm::vec3 getForward() const { return front; }
    glm::vec3 getUp() const { return up; }
    glm::vec3 getRight() const { return right; }
    glm::vec3 getEye() const { return eye; }

    f32 getAspectRatio() const { return aspectRatio; }
    f32 getFov() const { return glm::radians(fovDeg); }
    f32 getNear() const { return zmin; }
    f32 getFar() const { return zmax; }

    void lookAt(glm::vec3 newEye, glm::vec3 newTarget, glm::vec3 newUp = glm::vec3(0.0f, 1.0f, 0.0f));
    void setProjectionParams(f32 newFovDeg, f32 newAspectRatio, f32 near, f32 far);
    void setOrthoParams(bool isCentered, f32 left, f32 width, f32 top, f32 height, f32 near, f32 far);
    void setAspectRatio(f32 newAspectRatio);
};