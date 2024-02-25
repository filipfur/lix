#version 330 core

precision highp float;
in vec2 texCoords;
out vec4 fragColor;

uniform sampler2D u_texture;
uniform sampler2D u_bright;

void main()
{
    vec4 texDiffuse = texture(u_texture, texCoords);

    vec3 bloom = texture(u_bright, texCoords).rgb;

    vec3 color = texDiffuse.rgb + bloom;
    //vec3 color = texDiffuse.rgb + vec3(1,0.1,0.4) * bloom;

    fragColor = vec4(color, 1.0);
    const float exposure = 1.0;
    // reinhard tone mapping
    fragColor.rgb = vec3(1.0) - exp(-fragColor.rgb * exposure);

    // gamma correction
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
    //fragColor.rgb = texBright.rrr;
}