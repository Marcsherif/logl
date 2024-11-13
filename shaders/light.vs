#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0f));
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // in real application, calculate the normal matrix on the cpu and send it
    // via a uniform, since inverse operation is costly.
    gl_Position = projection * view * vec4(FragPos, 1.0);
};
