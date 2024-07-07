#version 330 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoords;
layout (location=3) in uvec4 aJoints;
layout (location=4) in vec4 aWeights;

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
layout (std140) uniform JointBlock
{
    mat4 u_jointMatrix[24];
};

uniform mat4 u_model;

void main()
{
    mat4 skinMatrix = u_jointMatrix[int(aJoints.x)] * aWeights.x
    + u_jointMatrix[int(aJoints.y)] * aWeights.y
    + u_jointMatrix[int(aJoints.z)] * aWeights.z
    + u_jointMatrix[int(aJoints.w)] * aWeights.w;

    texCoords = aTexCoords;
    eyePos = u_eye_pos;

    mat4 world = u_model * skinMatrix;

    //normal = transpose(inverse(mat3(world))) * aNormal;
    normal = normalize(mat3(world) * aNormal);

    //position = vec3(world * vec4(aPos.xyz, 1.0));
    position = vec3(world * vec4(aPos.xyz, 1.0));

    gl_Position = u_projection * u_view * vec4(position, 1.0);
}