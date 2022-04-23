#pragma once

#include <array>
#include <vector>
#include <string>
#include <map>

// Externals files
#include "external/glm/glm.hpp"
#include "external/glm/gtc/quaternion.hpp"
#include "external/glm/gtx/matrix_decompose.hpp"
#include "external/imgui/imgui.h"
#include "external/imgui/ImGuizmo.h"
#include "external/json/json.hpp"
using json = nlohmann::json;

// Application files
#include "defines.h"
#include "systems/handle/handle.h"
#include "core/utils.h"
#include "core/assert.h"
#include "core/logger.h"