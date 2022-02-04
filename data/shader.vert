#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
} ubo;

layout (push_constant) uniform pushConstant
{
    mat4 model;
} constant;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out vec3 worldNormal;

void main()
{
    mat4 model = constant.model;

    // Out values
    fragColor = color;
    outUV = uv;
    worldPos = (model * vec4(position, 1.0)).xyz;
    worldNormal = (model * vec4(normal, 1.0)).xyz;

    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);
}