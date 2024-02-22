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

uniform float u_time;
uniform vec3 u_lights[3];

const vec3 lightColors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.9, 0.7),
    vec3(0.4, 1.0, 1.0)
);

void main()
{
    vec4 texDiffuse = texture(u_texture, texCoords);
    if(texDiffuse.a < 0.1)
    {
        discard;
    }
    fragColor = texDiffuse * u_base_color;

    const float _Bias = 0.005;
    const float _Scale = 0.2;
    const int _Power = 2;

    vec3 I = normalize(position - eyePos.xyz);
	float R = _Bias + _Scale * pow(1.0 + dot(I, normal), float(_Power));

    fragColor.rgb = mix(fragColor.rgb, vec3(1.0, 0.0, 1.0), R);

    for(int i = 0; i < 3; ++i)
    {
       fragColor.rgb += lightColors[i] * (0.4 + sin(u_time * 8.0) * 0.05) * max(1.0 - length(position - u_lights[i]) * 0.15, 0.0);
    }

    colorBright = vec4(step(1.5, max(0.0, dot(fragColor.rgb, vec3(1,1,0)))) * fragColor.rgb,
        1.0);
    //fragColor.rgb = color;
}