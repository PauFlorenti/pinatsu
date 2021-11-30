#pragma once

#include "math_types.h"
#include "math.h"

#include "memory/pmemory.h"

vec3 operator/(const vec3& v, f32 f) {return {v.x / f, v.y / f, v.z / f};}

inline f32 vec3Length(const vec3& in)
{
    return in.x * in.x + in.y * in.y + in.z * in.z;
}

inline vec3 vec3Normalize(const vec3& in)
{
    vec3 out = in;
    f32 length = vec3Length(in);
    return out / length; 
}

inline f32 vec3Dot(const vec3& a, const vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 vec3Cross(const vec3& a, const vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline mat4 mat4Identity()
{
    mat4 out;
    memZero(out.m, sizeof(f32) * 16);
    out.m[0] = 1.0f;
    out.m[5] = 1.0f;
    out.m[10] = 1.0f;
    out.m[15] = 1.0f;
    return out;
}

inline mat4 mat4Perspective(
    f32 fov,
    f32 aspectRatio,
    f32 near,
    f32 far)
{
    f32 radFov = (fov * PI) / 180;
    f32 halfTanFov = tan(radFov * 0.5f);
    mat4 out;
    memZero(out.m, sizeof(f32) * 16);
    out.m[0] = 1.0f / (aspectRatio * halfTanFov);
    out.m[5] = 1.0f / halfTanFov;
    out.m[10] = -((far + near) / (far - near));
    out.m[11] = -1.0f;
    out.m[14] = -((2.0f * far * near) / (far - near));
    return out;
}

inline mat4 mat4LookAt(
    const vec3& position,
    const vec3& target,
    const vec3& up)
{
    mat4 out;
    vec3 zAxis = {
        target.x - position.x,
        target.y - position.y,
        target.z - position.z};

    zAxis = vec3Normalize(zAxis);
    vec3 xAxis = vec3Normalize(vec3Cross(zAxis, up));
    vec3 yAxis = vec3Cross(xAxis, zAxis);

    out.m[0] = xAxis.x;
    out.m[1] = yAxis.x;
    out.m[2] = -zAxis.x;
    out.m[3] = 0;
    out.m[4] = xAxis.y;
    out.m[5] = yAxis.y;
    out.m[6] = zAxis.y;
    out.m[7] = 0;
    out.m[8] = xAxis.z;
    out.m[9] = yAxis.z;
    out.m[10] = zAxis.z;
    out.m[11] = 0;
    out.m[12] = -vec3Dot(xAxis, position);
    out.m[13] = -vec3Dot(yAxis, position);
    out.m[14] = vec3Dot(zAxis, position);
    out.m[15] = 1.0f;

    return out;

}