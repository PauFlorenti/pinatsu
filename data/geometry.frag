#version 460

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inUV;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void main()
{
    outPosition = vec4(inWorldPos, 1.0);
    outNormal = (vec4( inNormal * 0.5 + vec3(0.5), 1));
    outAlbedo = vec4(inColor, 1.0);
}