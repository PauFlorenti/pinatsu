/**
 * Helper Functions for loading information via JSON
 * helping the data from components.
 */

#pragma once

glm::vec2 loadVec2(const std::string& str);
glm::vec2 loadVec2(const json& j, const char* attr);
glm::vec3 loadVec3(const std::string& str);
glm::vec3 loadVec3(const json& j, const char* attr);
glm::vec4 loadVec4(const std::string& str);
glm::vec4 loadVec4(const json& j, const char* attr);
glm::quat loadQuat(const json& j, const char* attr);

glm::vec4 loadColor(const json& j);
glm::vec4 loadColor(const json& j, const char* attr);
glm::vec4 loadColor(const json& j, const char* attr, const glm::vec4 defaultValue); 