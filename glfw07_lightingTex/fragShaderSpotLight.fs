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

struct FlashLight
{
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform vec3 emissionControl;
uniform float time;

uniform DirectionalLight dl;
uniform FlashLight flash;

void main()
{
    // Light affect the texture's color (ambinet, diffuse and specular. 3 color componets of Texture Outcome)
    // Ambient (+ Directional Light)
    vec3 ambientD = dl.ambient * vec3(texture(material.diffuse, texCoord)); 
    vec3 ambientS = flash.ambient * vec3(texture(material.diffuse, texCoord)); 

    // spot light diffuse
    vec3 normal = normalize(Normal);
    vec3 lightDirS = normalize(flash.position - FragPos); // Dir towards to light source
    float diff = max(dot(normal,lightDirS), 0.0f); // when the angle is bigger than pi/2, no light
    vec3 diffuseS = flash.diffuse * diff * vec3(texture(material.diffuse, texCoord)); // we use diffuseMap to replace the diffuse
    // directional light diffuse
    vec3 lightDirD = normalize(-dl.direction); // Directional Light towards to Fragment, we need to negate it
    diff = max(dot(normal,lightDirD),0.0f);
    vec3 diffuseD = dl.diffuse * diff * vec3(texture(material.diffuse, texCoord));

    // spot light specular, lightDirS (Direction of SpotLight), lightDirD (Direction of DirectionalLight)
    vec3 viewDir = lightDirS; // (flash.position - FragPos) == (viewerPos - FragPos) == viewerPos
    vec3 reflectDir = reflect(-lightDirS, normal); // this functions are available out of the box
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specularS = flash.specular * spec * vec3(texture(material.specular, texCoord)); // we also create a SpecularMap to replace specular
    // directional specular
    reflectDir = reflect(-lightDirD, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f);
    vec3 specularD = dl.specular * spec * vec3(texture(material.specular, texCoord));

    // vec3 emission = emissionControl * vec3(texture(material.emission, texCoord));
    vec3 emission = vec3(0.0f);
    if(texture(material.specular,texCoord).r == 0) // check the box's border. if current fragment to render is not on the specular border,
    {                                              // we render the emission texture
        // emission = emissionControl * vec3(texture(material.emission, texCoord));                        // flow along y-axis by time
        emission = emissionControl * vec3(texture(material.emission, texCoord + vec2(0.0f, time * 0.5f))); // repeat when the coordnate over 1.0
    }

    // Make spotLight has a soft edge
    vec3 spotDir = normalize(flash.direction); // towards to Fragment
    float theta = dot(-lightDirS, spotDir); // inner angle, both directions toward to fragments
    float epsilon = flash.cutOff - flash.outerCutOff; // 17.5 - 12.5 // outer angle - inner angle
    float intensity = clamp((theta - flash.outerCutOff)/epsilon,0.0f,1.0f); // we use two angles to control the Intensity AUTOMATICALLY
    vec3 diffAndSpeFromS = (diffuseS + specularS) * intensity; // spotLight will only influence Diffuse and Specular
    
    // attenuation only affects spotLight
    float flashDistance = length(flash.position - FragPos);
    float attenuation = 1.0/(flash.constant + flash.linear * flashDistance + flash.quadratic * (flashDistance * flashDistance));;
    ambientS *= attenuation;
    diffAndSpeFromS *= attenuation;

    vec3 dirResult = ambientD + diffuseD + specularD;
    vec3 spotResult = ambientS + diffAndSpeFromS;

    vec3 finalResult = dirResult + spotResult + emission; // ambient + diffuse + specular and emission
    finalResult /= (finalResult + vec3(1.0f)); // Normalize the final Result

    fragColor = vec4(finalResult, 1.0f);

}
