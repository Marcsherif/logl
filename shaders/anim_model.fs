#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse1;
    sampler2D specular1;
    sampler2D normal1;
};
uniform Material material;

void main()
{
    FragColor = texture(material.diffuse1, TexCoords);
}
