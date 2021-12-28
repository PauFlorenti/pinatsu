
/**
 * This system should not be called mesh loader, since it does not only load meshes
 * but it also creates default engine meshes such as triangles, planes and cubes given
 * the appropiate parameters.
 */

#pragma once

#include "systems/resourceSystem.h"

bool meshLoaderLoad(struct resourceLoader* self, const char* name, Resource* outResource);
void meshLoaderUnload(Resource* resource);