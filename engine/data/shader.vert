#version 450

vec2 position[3] = {
    { 0.0, -0.5},
    { 0.5,  0.5},
    {-0.5,  0.5}
};

void main()
{
    gl_Position = vec4(position[gl_VertexIndex], 0.0, 1.0);
}