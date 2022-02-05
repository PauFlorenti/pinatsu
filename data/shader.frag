
#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inWorldPos;
layout(location = 3) in vec3 inWorldNormal;

layout(set = 1, binding = 0) uniform Material
{
    vec4 diffuse;
} mat;

layout(set = 1, binding = 1) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 2) uniform sampler2D normalSampler;
layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessSampler;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 wPos = inWorldPos;
    vec3 wNorm = inWorldNormal;

    vec3 lightPos = vec3(1, 1, 1);
    vec3 L = (lightPos - wPos);
    float distanceToLight = length(L);
    L = normalize(L);

    float NdotL = dot(wNorm, L);

    vec4 diffuseTxt = texture(diffuseSampler, inUV);
    if(diffuseTxt.w < 1.0)
        discard;

    vec4 diffuse = mat.diffuse * diffuseTxt;

    fragColor = NdotL * diffuse;
    //fragColor = texture(normalSampler, inUV);
    //fragColor =  texture(metallicRoughnessSampler, inUV);
}