#version 460

layout(location = 0) in vec2 inUV;

const int MAX_LIGHTS = 16;
struct Light
{
    vec3 position;
    float intensity;
    vec3 color;
    float radius;
};

layout(set = 0, binding = 0) uniform sampler2D gbuf[3];
layout(set = 0, binding = 1) uniform LightBuffer
{
    Light[MAX_LIGHTS] l;
} lights;

layout(location = 0) out vec4 fragColor;

void main()
{
    // Basic information from previous passes
    vec3 albedo     = texture(gbuf[2], inUV).xyz;
    vec3 position   = normalize(texture(gbuf[0], inUV).xyz);
    vec3 N          = normalize(texture(gbuf[1], inUV).xyz * 2.0 - vec3(1));

    // Light calculations
    vec3 color = vec3(0);
    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        vec3 L = normalize(lights.l[i].position - position);
        float NdotL = dot(N, L);
        color += NdotL * lights.l[i].color;
    }

    color *= albedo;

    fragColor = vec4(color, 1.0);
}