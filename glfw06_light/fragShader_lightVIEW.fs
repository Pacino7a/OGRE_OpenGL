#version 330 core

// all of these are calculated in View Space
in vec3 FragPos;
in vec3 Normal;
in vec3 LightPos;

// so we makes the light in View Space
out vec4 fragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(normal,lightDir), 0.0f);
    vec3 diffuse = lightColor * diff;

    float ambientStrength = 0.2f;
    vec3 ambientLight = lightColor * ambientStrength;

    float specularStrength = 0.5f;
    vec3 viewDir = normalize(-FragPos); // viewerPos always be the origin in VIEW Space, so ViewDir is  fragPos' Reverse
    vec3 reflectDir = reflect(-lightDir, normal); 
    float spec = pow(max(dot(viewDir, reflectDir), 0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // ambient + diffuse + specular
    vec3 result = (diffuse + ambientLight + specular) * objectColor;
    fragColor = vec4(result, 1.0f);
}
