#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec2 outUV;

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec3 position;
} ubo;

layout (push_constant) uniform pushConstant
{
    mat4 model;
} constant;

void main()
{
    mat4 model = constant.model;
    vec3 worldPos = (model * vec4(position, 1.0)).xyz;
    outPosition = worldPos;
    outColor = color.xyz;
    outNormal = vec3(model * vec4(normal, 1.0));
    outUV = uv;
    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);
}