#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

void main()
{
    gl_Position = vec4(position, 1);
}