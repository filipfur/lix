#version 330 core

precision highp float;

out vec4 fragColor;

uniform vec3 u_rgb;

void main()
{
    fragColor = vec4(u_rgb, 0.7);
}