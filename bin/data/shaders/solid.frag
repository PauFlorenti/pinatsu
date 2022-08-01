
#version 460

layout(location = 0) in vec4 inColor;

layout(set = 1, binding = 0) uniform Material
{
    vec4 diffuse;
} mat;

layout(location = 0) out vec4 fragColor;

void main()
{
    // Diffuse color
    fragColor = inColor * mat.diffuse;
}