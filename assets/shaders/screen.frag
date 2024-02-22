#version 330 core

precision highp float;

in vec2 texCoords;
out vec4 fragColor;

uniform sampler2D u_texture;

void main()
{
    vec4 texDiffuse = texture(u_texture, texCoords);
    fragColor = texDiffuse;
}