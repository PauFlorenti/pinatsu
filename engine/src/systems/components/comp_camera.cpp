#include "comp_camera.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("camera", TCompCamera);

void TCompCamera::load(const json& j, TEntityParseContext& ctx)
{
    fovDeg = j.value("fov", fovDeg);
    zmin = j.value("near", zmin);
    zmax = j.value("far", zmax);
    setProjectionParams(fovDeg, 1.0f, zmin, zmax);
}

void TCompCamera::update(f32 dt)
{
    TCompTransform* cTransform = get<TCompTransform>();
    PASSERT(cTransform);
    glm::mat4 t = cTransform->asMatrix();
    const glm::vec3 fwd = glm::normalize(glm::vec3(t[2]));
    lookAt(cTransform->getPosition(), cTransform->getPosition() + fwd);
}

void TCompCamera::debugInMenu()
{

}