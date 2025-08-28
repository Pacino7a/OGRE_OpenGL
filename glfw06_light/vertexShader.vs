#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

// out vec3 vertexColor;
// out vec2 texCoord;

out vec3 FragPos;
out vec3 Normal;

// uniform mat4 transform;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 invModel; // let CPU to calculate this 


void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0f));
    Normal = mat3(transpose(invModel)) * aNormal; // using the normal matrix to handle the non-uniform scaling

    // Normal = aNormal;
    // vertexColor = aColor;
    // texCoord = aTexCoord;
}
