#version 460

layout(set = 0, binding = 0) uniform sampler2D text[3];
//layout(set = 0, binding = 1) uniform sampler2D normalTxt;
//layout(set = 0, binding = 2) uniform sampler2D albedoTxt;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(1, 0, 0, 1);
}