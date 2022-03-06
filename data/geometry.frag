#version 460

#extension GL_GOOGLE_include_directive : enable
#include "utils.glsl"

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inUV;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

layout(set = 1, binding = 0) uniform Material
{
    vec4 diffuse;
} mat;

layout(set = 1, binding = 1) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 2) uniform sampler2D normalSampler;
layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessSampler;

void main()
{
    vec3 wNorm  = inNormal;
    vec3 wPos   = inWorldPos;
    vec3 N      = normalize(texture(normalSampler, inUV).xyz);

    N = perturbNormal(wNorm, wPos, inUV, N);

    outPosition = vec4(inWorldPos, 1.0);
    outNormal = (vec4( N * 0.5 + vec3(0.5), 1));
    vec3 color = inColor * mat.diffuse.xyz;
    vec3 albedo = color * texture(diffuseSampler, inUV).xyz;
    outAlbedo = vec4(albedo, 1.0);
}