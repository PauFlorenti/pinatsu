#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
} ubo;

layout (push_constant) uniform pushConstant
{
    mat4 model;
} constant;

layout(location = 0) out vec4 fragColor;

void main()
{
    gl_Position = ubo.proj * ubo.view * constant.model * vec4(position, 1.0);
    fragColor = color;
}