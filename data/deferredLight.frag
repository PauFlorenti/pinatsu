#version 460

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D gbuf[3];

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 position = normalize(texture(gbuf[0], inUV).xyz);
    vec3 N = normalize(texture(gbuf[1], inUV).xyz * 2.0 - vec3(1));
    vec3 albedo = texture(gbuf[2], inUV).xyz;
    fragColor = vec4(albedo, 1.0);
}