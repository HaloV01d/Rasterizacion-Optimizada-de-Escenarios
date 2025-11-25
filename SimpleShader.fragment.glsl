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

uniform bool UseTexture;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(NormalMap, FragUV).xyz * 2.0 - 1.0;
    
    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(FragUV);
    vec2 st2 = dFdy(FragUV);
    
    vec3 N = normalize(FragNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    
    return normalize(TBN * tangentNormal);
}

void main()
{
    vec3 baseColor;
    float ao = 1.0;
    vec3 normal;
    float roughness = 0.5;
    
    if (UseTexture) {
        baseColor = texture(BaseColor, FragUV).rgb;
        normal = getNormalFromMap();
        ao = texture(AOMap, FragUV).r;
        roughness = texture(RoughnessMap, FragUV).r;
        
        // Ajuste de gamma para colores más vibrantes
        baseColor = pow(baseColor, vec3(0.9));
    } else {
        baseColor = MaterialColor;
        normal = normalize(FragNormal);
    }
    
    vec3 lightDir = normalize(LightDir);
    
    // Difusa mejorada
    float diff = max(dot(normal, lightDir), 0.0);
    diff = pow(diff, 0.8); // Suavizar transición de luz/sombra
    vec3 diffuse = diff * LightColor;
    
    // Especular mejorado (Blinn-Phong)
    vec3 viewDir = normalize(-FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), mix(8.0, 64.0, 1.0 - roughness));
    vec3 specular = spec * LightColor * (1.0 - roughness) * 0.4;
    
    // Rim light (luz de contorno) para dar más profundidad
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = pow(rim, 3.0);
    vec3 rimLight = rim * LightColor * 0.15;
    
    // Combinar todo
    vec3 ambient = AmbientColor * ao;
    vec3 result = (ambient + diffuse + specular + rimLight) * baseColor;
    
    // Tone mapping simple para mejor rango dinámico
    result = result / (result + vec3(1.0));
    
    // Gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}
