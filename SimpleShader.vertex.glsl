#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 ex_Normal;
out vec3 ex_Position;

void main()
{
    vec4 worldPos = ModelMatrix * vec4(in_Position, 1.0);
    ex_Position = worldPos.xyz;
    ex_Normal   = mat3(ModelMatrix) * in_Normal;

    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}
