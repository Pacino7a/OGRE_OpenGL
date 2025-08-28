#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 invModel;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0f));
    TexCoords = aTexCoord;

    mat3 normalMatrix = mat3(transpose(invModel));
    Normal = normalMatrix * aNormal; // using the normal matrix to handle the non-uniform scaling

    gl_PointSize = gl_Position.z;
    
    // if you have Normal Texture You can use TBN
    // vec3 T = normalize(normalMatrix * aTangent);
    // vec3 B = normalize(normalMatrix * aBitangent);
    // vec3 N = normalize(normalMatrix * aNormal);
 
    // TBN = mat3(T, B, N); // Get Enhanced Normal in World Space
}
