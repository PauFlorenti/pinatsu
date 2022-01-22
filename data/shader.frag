
#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;

layout(set = 1, binding = 0) uniform Material
{
    vec4 diffuse;
} mat;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = mat.diffuse;
}