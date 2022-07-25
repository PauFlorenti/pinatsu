#version 450

struct Camera
{
    vec3 position;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    mat4 viewprojection;
    mat4 inverse_viewprojection;
    vec3 position;
} ubo;

layout (push_constant) uniform pushConstant
{
    mat4 model;
} constant;

layout(location = 0) out vec4 fragColor;

void main()
{
    mat4 model = constant.model;

    // Out values
    vec3 worldPos = (model * vec4(position, 1.0)).xyz;
    fragColor = color;

    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);
}