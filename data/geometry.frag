#version 460

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void main()
{
    outPosition = vec4(1, 0, 0, 1);
    outNormal = vec4(0, 1, 0, 1);
    outAlbedo = vec4(0, 0, 1, 1);
}