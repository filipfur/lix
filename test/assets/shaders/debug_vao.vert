#version 330 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;

out vec3 position;

layout (std140) uniform CameraBlock
{
    mat4 u_projection;
    mat4 u_view;
    vec3 u_eye_pos;
};
uniform mat4 u_model;

void main()
{
  position = vec3(u_model * vec4(aPos, 1.0));
  gl_Position = u_projection * u_view * vec4(position, 1.0);
}