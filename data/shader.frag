
#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = inColor;
}