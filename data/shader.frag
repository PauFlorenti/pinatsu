
#version 450

const int MAX_LIGHTS = 16;

struct Light
{
    vec3 position;
    float intensity;
    vec3 color;
    float radius;
};

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inWorldPos;
layout(location = 3) in vec3 inWorldNormal;

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

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( p ).xy;
    vec2 duv2 = dFdy( p ).xy;

    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt( max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}

vec3 perturbNormal(vec3 N, vec3 WP, vec2 uv, vec3 normal_pixel)
{
    normal_pixel = normal_pixel * 255./127. - 128./127.;
    mat3 TBN = cotangent_frame(N, WP, uv);
    return normalize(TBN * normal_pixel);
}

void main()
{
    vec3 wPos = inWorldPos;
    vec3 wNorm = inWorldNormal;
    vec3 N = vec3(0.0);
    vec4 color = vec4(0.0);

    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        vec3 lightPos = lights.l[i].position;
        vec3 L = (lightPos - wPos);
        float distanceToLight = length(L);
        L = normalize(L);

        //if(distanceToLight > lights.l[i].radius)
        //    continue;

        float att_factor = 1.0 / distanceToLight;
        att_factor = 1.0;

        N = normalize(texture(normalSampler, inUV).xyz);
        //N = N * 2.0 - 1.0;
        N = perturbNormal(wNorm, wPos, inUV, N);
        float NdotL = dot(N, L);
        vec4 diffuseTxt = texture(diffuseSampler, inUV);
        if(diffuseTxt.w < 1.0)
            discard;
        vec4 diffuse = mat.diffuse * diffuseTxt;
        vec4 light = (NdotL * vec4(lights.l[i].color, 1.0)) * att_factor;
        color += light * diffuse;
    }
    //N = wNorm * 2.0 - 1.0;
    fragColor = color;
    //fragColor = vec4(N, 1.0);
}