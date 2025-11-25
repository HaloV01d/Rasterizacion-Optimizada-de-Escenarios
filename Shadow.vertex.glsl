#version 430 core

layout(location = 0) in vec3 in_Position;

uniform mat4 LightSpaceMatrix;
uniform mat4 ModelMatrix;

void main()
{
    gl_Position = LightSpaceMatrix * ModelMatrix * vec4(in_Position, 1.0);
}