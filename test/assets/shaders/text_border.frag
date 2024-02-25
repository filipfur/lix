#version 330 core

precision highp float;

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D u_texture;
uniform vec4 u_color;
uniform vec4 u_border_color;
uniform float u_border_width;
uniform float u_edge_smoothness;

void main()
{    
    // Retrieve the alpha value from the SDF texture
    float sdfValue = texture(u_texture, texCoord).r;

    // Calculate the smoothed alpha value for the text itself
    float border = u_edge_smoothness / 2.0;
    float alpha = smoothstep(0.5 - border, 0.5 + border, sdfValue);

    // Calculate the smoothed alpha value for the border
    float borderAlpha = smoothstep(0.5 - border - u_border_width, 0.5 - border, sdfValue);

    // Mix the text color and the border color based on the border alpha
    vec3 finalColor = mix(u_border_color.rgb, u_color.rgb, borderAlpha);

    // Set the final color of the fragment
    FragColor = vec4(finalColor, borderAlpha);
}