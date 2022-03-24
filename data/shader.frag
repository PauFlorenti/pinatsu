
#version 460
#extension GL_GOOGLE_include_directive : enable

#include "utils.glsl"
#include "pbr_funcs.glsl"

const int MAX_LIGHTS = 16;

struct Light
{
    vec3 position;
    float intensity;
    vec3 color;
    float radius;
    vec3 forward;
    float cosineCutoff;
    float spotExponent;
    bool enabled;
    int type;
    float dummy;
};

vec4 ComputeLight(in Light l, in vec3 N, in vec3 pos)
{
    return vec4(1);
}

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inWorldPos;
layout(location = 3) in vec3 inWorldNormal;
layout(location = 4) in vec3 inCamPosition;

layout(set = 0, binding = 1) uniform LightBuffer
{
    Light[MAX_LIGHTS] l;
} lights;

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
    vec3 wPos   = inWorldPos;
    vec3 wNorm  = inWorldNormal;
    vec3 N      = vec3(0.0);
    float ambient_factor = 0.1;
    vec3 V = normalize(inCamPosition - wPos);
    float NdotV = max(dot(N, V), 0.0);

    // Diffuse color
    vec4 diffuseTxt = texture(diffuseSampler, inUV);
    if(diffuseTxt.w < 1.0)
        discard;
    vec4 diffuse    = mat.diffuse * diffuseTxt;
    float metallic  = texture(metallicRoughnessSampler, inUV).z;
    float roughness = texture(metallicRoughnessSampler, inUV).y;
    vec3 F0         = mix(vec3(0.04), pow(diffuseTxt.xyz, vec3(2.2)), metallic);

    // Multipass lights
    vec4 light = vec4(0.0);
    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        Light l = lights.l[i];
        if(!lights.l[i].enabled)
            continue;

        vec3 lightPos = lights.l[i].position;
        vec3 L = (lightPos - wPos);
        float distanceToLight = length(L);
        L = normalize(L);
        vec3 H = normalize(V + L);

        if(distanceToLight >= lights.l[i].radius)
            continue;
        
        float lightIntensity = lights.l[i].intensity / (distanceToLight * distanceToLight);

        float att_factor = (lights.l[i].radius - distanceToLight) / lights.l[i].radius;
        att_factor = max(att_factor, 0.0);
        att_factor = att_factor * att_factor;

        vec3 radiance = lights.l[i].color * lightIntensity * att_factor;

        N = normalize(texture(normalSampler, inUV).xyz);
        N = perturbNormal(wNorm, wPos, inUV, N);
        float NdotL = dot(N, L);

        float NDF 	= DistributionGGX(N, H, roughness);
        float G 	= GeometrySmith(N, V, L, roughness);
        vec3 F 		= FresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 kD = vec3(1.0) - F;

        kD *= 1.0 - metallic;

        vec3 numerator 		= NDF * G * F;
        float denominator 	= 4.0 * NdotV * max(dot(N, L), 0.0);
        vec3 specular 		= numerator / max(denominator, 0.001);

        vec3 kS = F;

        light.xyz += (kD * pow(diffuseTxt.xyz, vec3(2.2)) / PI + specular) * radiance * NdotL;
    }
    light += vec4(ambient_factor);
    fragColor = light;
}