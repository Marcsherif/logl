#version 410 core

uniform float scale;
uniform float shift;

uniform float shaderR;
uniform float shaderG;
uniform float shaderB;

in float Height;

out vec4 FragColor;

void main()
{
    float h = (Height + shift)/scale;
    FragColor = vec4(shaderR*h, shaderG*h, shaderB*h, 1.0);
}
