#version 330 core
out float joint_weight;

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;

out vec3 frag_pos;
out vec3 normal;
out vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    joint_weight = 0.0;
    frag_pos = vec3(model * vec4(a_pos, 1.0f));
    normal = mat3(transpose(inverse(model))) * a_normal;
    tex_coords = vec2(a_tex_coords.x, 1-a_tex_coords.y);

    // in real application, calculate the normal matrix on the cpu and send it
    // via a uniform, since inverse operation is costly.
    gl_Position = projection * view * vec4(frag_pos, 1.0);
};
