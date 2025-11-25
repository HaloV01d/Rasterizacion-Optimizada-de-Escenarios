#version 430 core

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 LightDir;        // dirección de la luz
uniform vec3 LightColor;      // color de la luz
uniform vec3 AmbientColor;    // luz ambiente
uniform vec3 MaterialColor;   // color base del objeto

out vec4 out_Color;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-LightDir);  // luz que viene de LightDir

    float diff = max(dot(N, L), 0.0);

    vec3 color =
        AmbientColor * MaterialColor +
        diff * LightColor * MaterialColor;

    out_Color = vec4(color, 1.0);
}
