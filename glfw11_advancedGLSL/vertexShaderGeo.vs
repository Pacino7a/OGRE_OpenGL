#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (std140) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

out VS_OUT
{
    vec3 Normal;
    // vec2 TexCoords;
    mat4 Projection;

} vs_out;

uniform mat4 model;
uniform mat4 invModView;

// void main()
// {
//     gl_Position = projection * view * model * vec4(aPos, 1.0);
//     
//     // vs_out.TexCoords = aTexCoords;
// }

void main() // virtualization Normals
{
    gl_Position = view * model * vec4(aPos, 1.0); // don't mutiply projection
    // aNormal is in Local Space, normalMatrix(invModel) will transform it into World Space (usual case)
    mat3 normalMatrix = mat3(transpose(invModView)); // Because we will create another vertex by Normal in Geometry shader which is operating in View Space,
                                                     // so aNormal should transform into View Space
    vs_out.Normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
    vs_out.Projection = projection;
}
