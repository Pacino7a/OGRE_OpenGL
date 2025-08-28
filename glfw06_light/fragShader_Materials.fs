#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 fragColor;

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shiness;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// uniform vec3 lightColor;
// uniform vec3 lightPos;
uniform vec3 viewerPos;

uniform Material material;
uniform Light light;

void main()
{
    vec3 normal = normalize(Normal);

    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
    vec3 diffuse = light.diffuse * (diff * material.diffuse) ;

    vec3 ambient = light.ambient * material.ambient;

    vec3 viewDir = normalize(viewerPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specular = light.specular * (spec * material.specular);

    // ambient + diffuse + specular
    vec3 result = diffuse + ambient + specular;
    fragColor = vec4(result, 1.0f);
}
