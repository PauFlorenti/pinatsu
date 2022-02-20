#version 460

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D gbuf[3];

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = texture(gbuf[1], inUV);
}