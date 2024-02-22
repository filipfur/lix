#version 330 core

precision highp float;

in vec3 position;
layout (location=0) out vec4 fragColor;
layout (location=1) out vec4 colorBright;

void main()
{
    fragColor = vec4(1, 0, 0, 1);
    colorBright = vec4(1, 1, 1, 1);
}