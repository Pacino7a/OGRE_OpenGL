#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scrMap;

void main()
{
    vec2 brdf = texture(scrMap, TexCoords).rg;

    FragColor = vec4(brdf, 0.0, 1.0);
}
