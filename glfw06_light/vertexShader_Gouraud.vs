#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 invModel;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewerPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    vec3 Normal = mat3(transpose(invModel)) * aNormal; // using the normal matrix to handle the non-uniform scaling
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - aPos);
    float diff = max(dot(normal,lightDir), 0.0f);
    vec3 diffuse = lightColor * diff;

    float ambientStrength = 0.2f; // to create a simple ambient, all we need is creating an ambientStrength
    vec3 ambientLight = lightColor * ambientStrength;

    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewerPos - aPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0), 32); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specular = specularStrength * spec * lightColor;

    // ambient + diffuse + specular
    vertexColor = (diffuse + ambientLight + specular) * objectColor;
}
