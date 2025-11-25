#version 430 core

in vec3 ex_Normal;
in vec3 ex_Position;

out vec4 out_Color;

void main()
{
    vec3 N = normalize(ex_Normal);
    vec3 L = normalize(vec3(0.5, 1.0, 0.3)); // dirección de la luz

    float diff = max(dot(N, L), 0.0);

    vec3 baseColor = vec3(0.6, 0.8, 1.0);
    vec3 color = baseColor * (0.2 + 0.8 * diff); // algo de ambiente + difusa

    out_Color = vec4(color, 1.0);
}
