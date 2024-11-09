#version 330 core

in vec3 ourPos;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float alpha;

void main()
{
    vec4 tex2 = texture(texture2, vec2(1.0 - TexCoord.x, TexCoord.y));
    FragColor = mix(texture(texture1, TexCoord), tex2, alpha);
}
