#version 410 core

layout(vertices = 2) out;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    gl_TessLevelOuter[0] = 33.0; // Intel
    gl_TessLevelOuter[1] = 33.0; // GeForce
}


