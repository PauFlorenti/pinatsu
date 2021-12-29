#pragma once

#include "defines.h"
#include "resources/resourcesTypes.h"

// TODO dont use vector in the future
#include <vector>
#include "external/glm/glm.hpp"

typedef struct Entity
{
    Mesh* mesh;
    glm::mat4 model;
} Entity;

typedef struct Scene
{
    // TODO should derive to entities
    std::vector<Entity> entities;
} Scene;