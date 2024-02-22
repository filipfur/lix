#version 330 core

precision highp float;

in vec2 texCoords;
in vec3 color;
in vec3 position;
in vec3 normal;
in vec3 eyePos;
layout (location=0) out vec4 fragColor;
layout (location=1) out vec4 colorBright;

uniform sampler2D u_texture;
uniform vec4 u_base_color;

void main()
{
    fragColor = u_base_color;

    fragColor.rgb += texture(u_texture, texCoords).rgb;

    colorBright = vec4(step(1.5, max(0.0, dot(fragColor.rgb, vec3(1,1,0)))) * fragColor.rgb,
        1.0);
    //fragColor.rgb = color;
}