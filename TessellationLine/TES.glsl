#version 410 core

layout(isolines) in;

void main()
{
    vec3 pos1 = gl_in[0].gl_Position.xyz;
    vec3 pos2 = gl_in[1].gl_Position.xyz;
    float interp = gl_TessCoord.x;
    
    gl_Position = vec4(mix(pos1.x, pos2.x, interp), sin(2 * 3.1415926 * interp), 0.0, 1.0);
}


