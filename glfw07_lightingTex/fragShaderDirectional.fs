#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 texCoord;

out vec4 fragColor;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shiness;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewerPos;
uniform Material material;
uniform Light light;
uniform vec3 emissionControl;
uniform float time;

uniform DirectionalLight Dlight;

void main()
{
    // Light is a DIRECTIONAL LIGHT!, not a point light
    vec3 ambient = Dlight.ambient * vec3(texture(material.diffuse, texCoord)); // Ambient

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-Dlight.direction); // Directional Light is in everywhere, so lightDir is nothing to do with FragPos.
                                                  // but we expect it toward to Light Source, so we negate it
    
    float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
    vec3 diffuse = Dlight.diffuse * diff * vec3(texture(material.diffuse, texCoord)); // we use diffuseMap to replace the diffuse

    vec3 viewDir = normalize(viewerPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specular = Dlight.specular * spec * vec3(texture(material.specular, texCoord)); // we also create a SpecularMap to replace specular

    // vec3 emission = emissionControl * vec3(texture(material.emission, texCoord));
    vec3 emission = vec3(0.0f);
    if(texture(material.specular,texCoord).r == 0) // check the box's border. if current fragment to render is not on the specular border,
    {                                              // we render the emission texture
        // emission = emissionControl * vec3(texture(material.emission, texCoord));                        // flow along y-axis by time
        emission = emissionControl * vec3(texture(material.emission, texCoord + vec2(0.0f, time * 0.5f))); // repeat when the coordnate over 1.0
    }

    // ambient + diffuse + specular and emission
    vec3 result = diffuse + ambient + specular + emission;
    fragColor = vec4(result, 1.0f);

}
