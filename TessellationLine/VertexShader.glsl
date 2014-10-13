#version 410 core

void main()
{
    vec4 pos = vec4(0.0, 0.0, 0.0, 1.0);
    if(gl_VertexID == 0)
    {
        pos.xy = vec2(-1.0, 0.0);
    }
    else if(gl_VertexID == 1)
    {
        pos.xy = vec2(1.0, 0.0);
    }
    else if(gl_VertexID == 2)
    {
        pos.xy = vec2(0.0, 1.0);
    }
    gl_Position = pos;
}


