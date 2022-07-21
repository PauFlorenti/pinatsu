#version 460

#extension GL_GOOGLE_include_directive : enable
#include "pbr_funcs.glsl"

layout(location = 0) in vec2 inUV;

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
    float dummyValue;
};

layout(set = 0, binding = 0) uniform sampler2D gbuf[3];
layout(set = 0, binding = 1) uniform LightBuffer
{
    Light[MAX_LIGHTS] l;
} lights;

layout(set = 0, binding = 2) uniform cameraInfo {
    mat4 view;
    mat4 proj;
    mat4 viewprojection;
    mat4 inverse_viewprojection;
    vec3 position;
}camera;

layout(location = 0) out vec4 fragColor;

void main()
{
    // Basic information from previous passes
    vec3 albedo     = texture(gbuf[2], inUV).xyz;
    vec3 position   = texture(gbuf[0], inUV).xyz;
    vec3 N          = normalize(texture(gbuf[1], inUV).xyz * 2.0 - vec3(1));

    vec3 cameraPosition = camera.position;
    float ambient_factor = 0.1;

    vec3 V = normalize(cameraPosition - position);
    float NdotV = max(dot(N, V), 0.0);

    float metallic  = texture(gbuf[0], inUV).w;
    float roughness = texture(gbuf[1], inUV).w;
    vec3 F0         = mix(vec3(0.04), pow(albedo, vec3(2.2)), metallic);

    // Light calculations
    vec4 light = vec4(0.0);
    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        if(!lights.l[i].enabled || lights.l[i].intensity < 0.1)
            continue;

        vec3 lightPos   = lights.l[i].position;
        vec3 L          = (lightPos - position);
        float distanceToLight = length(L);
        L = normalize(L);
        vec3 H = normalize(V + L);

        if(distanceToLight > lights.l[i].radius)
            continue;

        float lightIntensity = lights.l[i].intensity / (distanceToLight * distanceToLight);

        float att_factor = (lights.l[i].radius - distanceToLight) / lights.l[i].radius;
        att_factor = max(att_factor, 0.0);
        att_factor = att_factor * att_factor;

        vec3 radiance = lights.l[i].color * lightIntensity * att_factor;

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

        light.xyz += (kD * pow(albedo, vec3(2.2)) / PI + specular) * radiance * NdotL * lightIntensity;
    }

    //light += vec4(ambient_factor);
    fragColor = light;
}