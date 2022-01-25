
#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;

layout(set = 1, binding = 0) uniform Material
{
    vec4 diffuse;
} mat;

layout(set = 1, binding = 1) uniform sampler2D diffuseSampler;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 diffuse = texture(diffuseSampler, inUV);
    if(diffuse.w < 1.0)
        discard;

    fragColor = mat.diffuse * diffuse;
}