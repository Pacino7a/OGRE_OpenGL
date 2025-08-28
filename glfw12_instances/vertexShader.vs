#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec2 aOffset;

// layout (std140) uniform Matrices
// {
//     mat4 view; // 4 * 16 Bytes
//     mat4 projection; // 4 * 16 Bytes
// };

out VS_OUT
{
    // vec3 FragPos;
    // vec2 TexCoords;
    // vec3 Normal;
    vec3 Color;

} vs_out;

// uniform mat4 model;
// uniform mat4 invModel;

// uniform vec2 offset[100]; // uniform has a small size Limitation! (100)

void main()
{
    // gl_Position = projection * view * model * vec4(aPos, 1.0);
    // vs_out.FragPos = vec3(model * vec4(aPos, 1.0f));
    // vs_out.TexCoords = aTexCoord;

    // gl_Position = vec4(aPos+offset[gl_InstanceID], 0.0, 1.0); // uniform has a small size Limitation!
    // vec2 pos = aPos * (gl_InstanceID / 100.0f);
    gl_Position = vec4(aPos+aOffset, 0.0, 1.0);
    vs_out.Color = aColor;

    // gl_PointSize = gl_Position.z;

    // mat3 normalMatrix = mat3(transpose(invModel));
    // vs_out.Normal = normalMatrix * aNormal; // using the normal matrix to handle the non-uniform scaling

    
    // if you have Normal Texture You can use TBN

    // vec3 T = normalize(normalMatrix * aTangent);
    // vec3 B = normalize(normalMatrix * aBitangent);
    // vec3 N = normalize(normalMatrix * aNormal);
 
    // TBN = mat3(T, B, N); // Get Enhanced Normal in World Space
}
