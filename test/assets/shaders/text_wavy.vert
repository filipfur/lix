#version 330 core

layout (location = 0) in vec4 aVertex;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform float u_time;

out vec2 texCoord;
out vec2 position;

void main()
{
    position = vec2(aVertex.x, -aVertex.y);
    texCoord = aVertex.zw;
    vec3 p0 = vec3(u_model * vec4(position, 0.0, 1.0));
    p0.y += cos(position.x + u_time * 8.0) * 6.0;
    gl_Position = u_projection * u_view * vec4(p0, 1.0);
}