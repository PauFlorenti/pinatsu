#pragma once

#include "defines.h"
#include "resources/resourcesTypes.h"

// TODO dont use vector in the future
#include <vector>

typedef struct Scene
{
    // TODO should derive to entities
    std::vector<Mesh*> meshes;
} Scene;