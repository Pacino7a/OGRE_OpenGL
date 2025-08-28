#version 330 core
// layout (depth_greater) out float gl_FragDepth; // for modifying the fragDepth Value

in VS_OUT
{
    // vec3 Normal;
    vec3 FragPos;
    vec2 TexCoords;

} fs_in;


out vec4 FragColor;

uniform sampler2D texture1;


void main()
{
    FragColor = texture(texture1, fs_in.TexCoords);
}
