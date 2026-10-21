#version 410 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;

out vec2 tex_coord;

uniform mat4 model;

void main()
{
    tex_coord = a_tex;
    gl_Position = vec4(a_pos, 1.0);
}
