
/**
 * This system should not be called mesh loader, since it does not only load meshes
 * but it also creates default engine meshes such as triangles, planes and cubes given
 * the appropiate parameters.
 */

#pragma once

#include "systems/resourceSystem.h"

resourceLoader meshLoaderCreate();