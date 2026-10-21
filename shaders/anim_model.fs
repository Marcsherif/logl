#version 330 core
out vec4 frag_color;

in vec2 tex_coords;

uniform vec3 view_pos;

struct Material {
    sampler2D diffuse1;
    sampler2D specular1;
    sampler2D normal1;
};
uniform Material material;

void main()
{
    frag_color = texture(material.diffuse1, tex_coords);
}
