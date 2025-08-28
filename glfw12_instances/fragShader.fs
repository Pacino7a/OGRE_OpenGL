#version 330 core

in VS_OUT
{
    // vec3 FragPos;
    // vec2 TexCoords;
    // vec3 Normal;
    vec3 Color;

} fs_in;


out vec4 FragColor;

// uniform sampler2D texture1;


void main()
{
    FragColor = vec4(fs_in.Color, 1.0f);
}
