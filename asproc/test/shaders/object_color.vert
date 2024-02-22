#version 330 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoords;
layout(location=2) in vec3 aColor;

out vec2 texCoords;
out vec3 color;
uniform float u_time;

void main()
{
  texCoords = aTexCoords;
  color = aColor;
  gl_Position = vec4(aPos + vec3(vec2(sin(u_time) * 0.2), 0.0), 1.0); 
}