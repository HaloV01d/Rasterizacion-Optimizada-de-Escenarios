#version 430 core

in vec3 FragNormal;
in vec3 FragPos;
in vec2 FragUV;

out vec4 FragColor;

uniform vec3 LightDir;
uniform vec3 LightColor;
uniform vec3 AmbientColor;
uniform vec3 MaterialColor;

uniform sampler2D BaseColor;
uniform sampler2D NormalMap;
uniform sampler2D RoughnessMap;
uniform sampler2D AOMap;

uniform bool UseTexture; // Nuevo uniform

void main()
{
    vec3 norm = normalize(FragNormal);
    vec3 lightDir = normalize(LightDir);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * LightColor;
    
    vec3 baseColor;
    float ao = 1.0;
    
    if (UseTexture) {
        // Usar texturas
        baseColor = texture(BaseColor, FragUV).rgb;
        ao = texture(AOMap, FragUV).r;
    } else {
        // Usar color sólido
        baseColor = MaterialColor;
    }
    
    vec3 result = (AmbientColor + diffuse) * baseColor * ao;
    
    FragColor = vec4(result, 1.0);
}
