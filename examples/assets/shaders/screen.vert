#version 330 core

layout(location=0) in vec4 aVertex;

out vec2 texCoords;

void main()
{
  texCoords = aVertex.zw;
  gl_Position = vec4(aVertex.xy, 0.0, 1.0);
}