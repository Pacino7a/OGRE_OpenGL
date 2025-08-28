#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;


out vec3 FragPos;
out vec3 Normal;
out vec3 LightPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 invModel; // glm::inverse(view * model)
uniform vec3 lightPos;


void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // all of them are transformed into View Space
    FragPos = vec3(view * model * vec4(aPos, 1.0f));
    Normal = mat3(transpose(invModel)) * aNormal;
    LightPos = vec3(view * vec4(lightPos, 1.0f));
}
