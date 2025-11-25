#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 vNormal;
out vec3 vWorldPos;

void main()
{
    vec4 worldPos = ModelMatrix * vec4(in_Position, 1.0);
    vWorldPos = worldPos.xyz;

    // Para no complicarnos con matrices inversas:
    vNormal = normalize(mat3(ModelMatrix) * in_Normal);

    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}
