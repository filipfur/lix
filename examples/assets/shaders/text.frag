#version 330 core

precision highp float;

out vec4 FragColor;

uniform vec4 u_color;
uniform sampler2D u_texture;

in vec2 texCoords;
in vec2 position;

void main()
{    
    float sdfValue = texture(u_texture, texCoords).r;

    float smoothing = 0.08;
    float border = smoothing / 2.0;
    float alpha = smoothstep(0.5 - border, 0.5 + border, sdfValue);

    FragColor = vec4(u_color.rgb, u_color.a * alpha);
}