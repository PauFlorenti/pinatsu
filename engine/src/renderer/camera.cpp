#include "camera.h"

void CCamera::updateViewProjection()
{
    viewProjection = view * projection;
}

void CCamera::lookAt(glm::vec3 newEye, glm::vec3 newTarget, glm::vec3 newUp)
{
    eye = newEye;
    target = newTarget;
    view = glm::lookAt(newEye, newTarget, newUp);
    updateViewProjection();

    front = glm::normalize(target - eye);
    right = -glm::normalize(glm::cross(newUp, front));
    up = glm::cross(front, -right);
}

void CCamera::setProjectionParams(f32 newFovDeg, f32 newAspectRatio, f32 near, f32 far)
{
    zmin = near;
    zmax = far;
    fovDeg = newFovDeg;
    aspectRatio = newAspectRatio;
    projection = glm::perspective(glm::radians(fovDeg), aspectRatio, zmin, zmax);
    updateViewProjection();

    isOrtho = false;
}

void CCamera::setOrthoParams(bool isCentered, f32 left, f32 width, f32 top, f32 height, f32 near, f32 far)
{
    zmin = near;
    zmax = far;
    if(isCentered)
    {
        projection = glm::ortho(left, width, height, top, zmin, zmax);
    }
    else
    {
        PWARN("Pending to implement and off centered orthographic camera.")
        // TODO 
    }
    updateViewProjection();
    isOrtho = true;
}

void CCamera::setAspectRatio(f32 newAspectRatio)
{
    setProjectionParams(fovDeg, newAspectRatio, zmin, zmax);
}