#version 330 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;

out vec2 texCoords;
out vec3 color;
out vec3 position;
out vec3 normal;
out vec3 eyePos;

layout (std140) uniform CameraBlock
{
    mat4 u_projection;
    mat4 u_view;
    vec3 u_eye_pos;
};
uniform mat4 u_model;

uniform vec2 u_vec2;
uniform ivec2 u_ivec2;
uniform vec3 u_vec3;
uniform ivec3 u_ivec3;
uniform mat3 u_mat3;

void main()
{
  texCoords = aTexCoords;
  mat4 world = u_model * mat4(u_mat3);
  position = vec3(world * vec4(aPos + u_vec3 + vec3(u_ivec3) + vec3(u_vec2 + vec2(u_ivec2), 0.0), 1.0));
  normal = normalize(mat3(world) * aNormal);
  eyePos = u_eye_pos;
  color = vec3(1.0);
  gl_Position = u_projection * u_view * vec4(position, 1.0);
}