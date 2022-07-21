#include "helper.h"

glm::vec2 loadVec2(const std::string& str)
{
    glm::vec2 v;
    int n = sscanf(str.c_str(), "%f %f", &v.x, &v.y);
    if(n == 2)
        return v;
    PFATAL("Invalid reading string for a VEC2. Reading only %d values, 2 expected.", n);
    return glm::vec2(1.0f);
}

glm::vec2 loadVec2(const json& j, const char* attr)
{
    PASSERT(j.is_object())
    if(j.count(attr))
    {
        const std::string& str = j.value(attr, "");
        return loadVec2(str);
    }
    return glm::vec2(0.0f);
}

glm::vec3 loadVec3(const std::string& str)
{
    glm::vec3 v;
    int n = sscanf(str.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
    if(n == 3)
        return v;
    PFATAL("Invalid str for a VEC3. Reading only %d values, 3 expected.", n)
    return glm::vec3(0.0f);
}

glm::vec3 loadVec3(const json& j, const char* attr)
{
    PASSERT(j.is_object());
    if(j.count(attr))
    {
        const std::string& str = j.value(attr, "");
        return loadVec3(str);
    }
    return glm::vec3(0.0f);
}

glm::vec4 loadVec4(const std::string& str)
{
    glm::vec4 v;
    int n = sscanf(str.c_str(), "%f %f %f %f", &v.x, &v.y, &v.z, &v.w);
    if(n == 4)
        return v;
    PFATAL("Invalid str for a VEC3. Reading only %d values. Expected 4.", n)
    return glm::vec4(0.0f);
}
glm::vec4 loadVec4(const json& j, const char* attr)
{
    PASSERT(j.is_object());
    if(j.count(attr))
    {
        const std::string& str = j.value(attr, "");
        return loadVec4(str);
    }
    return glm::vec4(0.0f);
}

glm::quat loadQuat(const json& j, const char* attr)
{
    PASSERT(j.is_object())
    if(j.count(attr))
    {
        const std::string str = j.value(attr, "");
        glm::quat q;
        int n = sscanf(str.c_str(), "%f %f %f %f", &q.x, &q.y, &q.z, &q.w);
        if(n == 4)
            return q;
        PFATAL("Invalid json reading QUAT attr %s. Only %d values red, 4 expected.", attr, n);
    }
    return glm::quat();
}

glm::vec4 loadColor(const json& j)
{
    glm::vec4 c;
    const auto& str = j.get<std::string>();
    int n = sscanf(str.c_str(), "%f %f %f %f", &c.x, &c.y, &c.z, &c.w);
    if(n == 4)
        return c;
    PFATAL("Invalid string reading for Color %s. %d value read, expected 4.", str.c_str(), n);
    return glm::vec4(1.0f);
}

glm::vec4 loadColor(const json& j, const char* attr)
{
    PASSERT(j.is_object())
    if(j.count(attr))
        return loadColor(j[attr]);
    return glm::vec4(1.0f);
}

glm::vec4 loadColor(const json& j, const char* attr, const glm::vec4& defaultValue)
{
    if(j.count(attr) <= 0)
        return defaultValue;
    return loadColor(j, attr);
}