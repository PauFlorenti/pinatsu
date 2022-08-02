
#version 460

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUv;

layout(set = 1, binding = 0) uniform Material
{
    vec4 diffuse;
} mat;

layout(set = 1, binding = 1) uniform sampler2D diffuseSampler;

layout(location = 0) out vec4 fragColor;

void main()
{
    // Diffuse color
    vec4 diffuse = texture(diffuseSampler, inUv);
    fragColor = inColor * mat.diffuse * diffuse;
}