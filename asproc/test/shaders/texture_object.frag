#version 330 core

precision highp float;
in vec2 texCoords;
in vec3 color;
out vec4 fragColor;

uniform sampler2D u_texture;

void main()
{
    vec4 texDiffuse = texture(u_texture, texCoords);
    fragColor = vec4(texDiffuse.rgb, 1.0);
}