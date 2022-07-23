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

/** 
 * Helper function for math and algebra.
 */

glm::vec3 yawToVector(f32 yaw);
/** Returns yaw in radians.*/
f32 vectorToYaw(glm::vec3 front);
glm::vec3 yawPitchToVector(f32 yaw, f32 pitch);
/** Return yaw and pitch in radians.*/
void vectorToYawPitch(glm::vec3 front, f32* yaw, f32* pitch);