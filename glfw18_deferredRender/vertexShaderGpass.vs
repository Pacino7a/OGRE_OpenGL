#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes
};

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 model;
uniform mat4 invModel;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    FragPos = vec3(model * vec4(aPos, 1.0f));
    TexCoords = aTexCoord;

    mat3 normalMatrix = mat3(transpose(invModel));
    Normal = normalMatrix * aNormal;
}
