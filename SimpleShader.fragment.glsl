#version 430 core

in vec3 FragNormal;
in vec3 FragPos;
in vec2 FragUV;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform vec3 LightDir;
uniform vec3 LightColor;
uniform vec3 AmbientColor;
uniform vec3 MaterialColor;
uniform vec3 ViewPos;

uniform sampler2D BaseColor;
uniform sampler2D ShadowMap;

uniform bool UseTexture;

// Calcular sombra MEJORADO
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perspectiva divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transformar de [-1,1] a [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    
    // Si está fuera del rango [0,1], no hay sombra
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0 ||
       projCoords.z > 1.0)
        return 0.0;
    
    // Obtener profundidad desde el shadow map
    float closestDepth = texture(ShadowMap, projCoords.xy).r;
    
    // Profundidad actual del fragmento
    float currentDepth = projCoords.z;
    
    // Bias para evitar shadow acne
    float bias = max(0.003 * (1.0 - dot(normal, lightDir)), 0.0005);
    
    // PCF (suavizado de sombras) - 3x3 kernel
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMap, 0);
    
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

void main()
{
    // Obtener color base
    vec3 baseColor;
    if (UseTexture) {
        baseColor = texture(BaseColor, FragUV).rgb;
        baseColor = pow(baseColor, vec3(2.2)); // Gamma correction
    } else {
        baseColor = MaterialColor;
    }
    
    // Normalizar vectores
    vec3 normal = normalize(FragNormal);
    vec3 lightDir = normalize(LightDir);
    vec3 viewDir = normalize(ViewPos - FragPos);
    
    // AMBIENTE
    vec3 ambient = AmbientColor * baseColor;
    
    // DIFUSA
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * LightColor * baseColor;
    
    // ESPECULAR (Phong)
    vec3 reflectDir = reflect(-lightDir, normal);
    float shininess = 32.0;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * LightColor * 0.5;
    
    // CALCULAR SOMBRA
    float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
    
    // COMBINAR (sombra NO afecta ambiente, solo difusa y especular)
    vec3 lighting = ambient + (1.0 - shadow * 0.85) * (diffuse + specular); // Sombras más oscuras (85%)
    
    // Gamma correction final
    vec3 result = pow(lighting, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}
