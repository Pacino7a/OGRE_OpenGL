#version 330 core

// in vec3 vertexColor;
// in vec2 texCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 fragColor;

// uniform sampler2D texture1;
// uniform sampler2D texture2;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewerPos;

void main()
{
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
    vec3 diffuse = lightColor * diff;

    float ambientStrength = 0.2f; // to create a simple ambient, all we need is creating an ambientStrength
    vec3 ambientLight = lightColor * ambientStrength;

    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewerPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
    float spec = pow(max(dot(viewDir, reflectDir), 0), 32); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specular = specularStrength * spec * lightColor;

    // ambient + diffuse + specular
    vec3 result = (diffuse + ambientLight + specular) * objectColor;
    fragColor = vec4(result, 1.0f);
    // fragColor = mix(texture(texture1, texCoord) ,texture(texture2, vec2(1-texCoord.x, texCoord.y)), 0.5f) * vec4(vertexColor, 1.0f);
    // fragColor = mix(texture(texture1, texCoord) ,texture(texture2, vec2(1-texCoord.x, texCoord.y)), 0.5f);
}
