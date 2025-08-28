#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 invModel;
uniform bool invNormal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0f));
    TexCoords = aTexCoord;

    mat3 normalMatrix = mat3(transpose(invModel));
    vec3 normal = invNormal? -aNormal : aNormal;
    Normal = normalMatrix * normal;
}
