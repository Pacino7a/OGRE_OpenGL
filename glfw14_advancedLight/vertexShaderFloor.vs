#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes
    bool blinn; // 4
    bool gamma; // 4
};

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 FragPosLightSpace;

} vs_out;

uniform mat4 model;
uniform mat4 invModel;
uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0f));
    vs_out.TexCoords = aTexCoord;

    mat3 normalMatrix = mat3(transpose(invModel));
    vs_out.Normal = normalMatrix * aNormal; // using the normal matrix to handle the non-uniform scaling

    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

}
