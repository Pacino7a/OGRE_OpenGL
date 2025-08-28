#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoords;
layout (std140) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

out VS_OUT
{
    // vec3 vertexColor;
    vec2 TexCoords;

} vs_out;

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    vs_out.TexCoords = aTexCoords;
}
