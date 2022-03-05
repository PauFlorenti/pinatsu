#version 450

struct Camera
{
    vec3 position;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec3 position;
} ubo;

layout (push_constant) uniform pushConstant
{
    mat4 model;
} constant;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out vec3 worldNormal;
layout(location = 4) out vec3 camPosition;

void main()
{
    mat4 model = constant.model;

    // Out values
    worldPos = (model * vec4(position, 1.0)).xyz;
    fragColor = color;
    outUV = uv;
    worldNormal = mat3(transpose(inverse(model))) * normal;
    camPosition = ubo.position;

    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);
}