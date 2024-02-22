#version 330 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;
layout(location=3) in vec3 aColor;

out vec2 texCoords;
out vec3 color;
uniform float u_time;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
  texCoords = aTexCoords;
  color = aColor;
  //gl_Position = vec4(aPos + vec3(vec2(sin(u_time) * 0.2), 0.0), 1.0);
  gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}