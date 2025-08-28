#version 330 core
layout (location = 0) in vec3 aPos;
layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes
};

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
