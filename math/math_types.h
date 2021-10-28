#pragma once

#include "defines.h"

typedef union vec2
{
    f32 data[2];
    struct
    {
        union{
            f32 x, r;
        };
        union{
            f32 y, g;
        };
    };
} vec2;

typedef union vec3
{
    f32 data[3];
    struct
    {
        union{
            f32 x, r;
        };
        union{
            f32 y, g;
        };
        union{
            f32 z, b;
        };
    };
} vec3;

typedef union vec4
{
    f32 data[4];
    struct
    {
        union{
            f32 x, r;
        };
        union{
            f32 y, g;
        };
        union{
            f32 z, b;
        };
        union{
            f32 w, a;
        };
    };
} vec4;

typedef vec4 quat;

typedef union mat4
{
    f32 m[16];
} mat4;