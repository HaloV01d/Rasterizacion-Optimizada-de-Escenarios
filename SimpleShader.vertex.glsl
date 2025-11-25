#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

out vec3 FragNormal;
out vec3 FragPos;
out vec2 FragUV;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    FragPos = vec3(ModelMatrix * vec4(in_Position, 1.0));
    FragNormal = mat3(transpose(inverse(ModelMatrix))) * in_Normal;
    FragUV = in_UV;
    
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(FragPos, 1.0);
}
