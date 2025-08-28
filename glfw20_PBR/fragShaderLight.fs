#version 330 core

out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
    vec3 lightColor_toneMap = lightColor / (lightColor + vec3(1.0));
    FragColor = vec4(lightColor_toneMap, 1.0f);
    // fragColor = vec4(1.0f);
}
