#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
} ubo;

layout (push_constant) uniform pushConstant
{
    mat4 model;
} constant;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 textCoord;

void main()
{
    gl_Position = ubo.proj * ubo.view * constant.model * vec4(position, 1.0);
    fragColor = color;
    textCoord = uv;
}