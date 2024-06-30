#version 330 core

precision highp float;

in vec2 texCoords;
in vec3 color;
in vec3 position;
in vec3 normal;
in vec3 eyePos;
out vec4 fragColor;

uniform sampler2D u_texture;
uniform vec4 u_base_color;

uniform float u_time;

void main()
{
    vec4 texDiffuse = texture(u_texture, texCoords);
    if(texDiffuse.a < 0.1)
    {
        discard;
    }
    fragColor = texDiffuse * u_base_color;
    fragColor = vec4(pow(fragColor.rgb, vec3(1.0 / 2.2)), 1.0);
}