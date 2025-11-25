#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

out vec3 FragNormal;
out vec3 FragPos;
out vec2 FragUV;
out vec4 FragPosLightSpace;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 LightSpaceMatrix;

void main()
{
    // Posición del fragmento en espacio mundial
    FragPos = vec3(ModelMatrix * vec4(in_Position, 1.0));
    
    // Normal transformada (sin traslación)
    FragNormal = mat3(transpose(inverse(ModelMatrix))) * in_Normal;
    
    // Coordenadas UV
    FragUV = in_UV;
    
    // Posición en espacio de luz para sombras
    FragPosLightSpace = LightSpaceMatrix * vec4(FragPos, 1.0);
    
    // Posición final del vértice
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(FragPos, 1.0);
}
