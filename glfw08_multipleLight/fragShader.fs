#version 330 core
#define NR_POINT_LIGHTS 4
// #define NR_SPOT_LIGHTS 4


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

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
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

uniform Light pointLights[NR_POINT_LIGHTS];
// uniform FlashLight flashLights[NR_SPOT_LIGHTS];

// uniform Light light;
uniform DirectionalLight dl;
uniform FlashLight flash;
uniform Material material;
uniform vec3 viewerPos;
uniform vec3 emissionControl;
uniform float time;

vec3 DirectionalEffect(vec3 FragPos, vec3 normal, vec2 texCoord)
{
    // Light is a DIRECTIONAL LIGHT!, not a point light
    vec3 ambient = dl.ambient * vec3(texture(material.diffuse, texCoord)); // Ambient

    vec3 lightDir = normalize(-dl.direction); // Directional Light is in everywhere, so lightDir is nothing to do with FragPos.
                                                  // but we expect it toward to Light Source, so we negate it
    float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
    vec3 diffuse = dl.diffuse * diff * vec3(texture(material.diffuse, texCoord)); // we use diffuseMap to replace the diffuse

    vec3 viewDir = normalize(viewerPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specular = dl.specular * spec * vec3(texture(material.specular, texCoord)); // we also create a SpecularMap to replace specular

    return ambient + diffuse + specular;
}

vec3 pointLightEffect(const Light light, vec3 FragPos, vec3 normal, vec2 texCoord)
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord)); // Ambient
    
    vec3 lightDir = normalize( light.position - FragPos );
    float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord)); // we use diffuseMap to replace the diffuse

    vec3 viewDir = normalize(viewerPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord)); // we also create a SpecularMap to replace specular

    // attenuation
    float lightDistance = length(light.position - FragPos);
    float attenuation = 1.0/(light.constant + light.linear * lightDistance + light.quadratic * (lightDistance * lightDistance));

    return (ambient + diffuse + specular) * attenuation;
}

vec3 spotLightEffect(const FlashLight flash, vec3 FragPos, vec3 normal, vec2 texCoord)
{
    vec3 ambient = flash.ambient * vec3(texture(material.diffuse, texCoord)); 

    // spot light diffuse
    vec3 lightDir = normalize(flash.position - FragPos); // Dir towards to light source
    float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
    vec3 diffuse = flash.diffuse * diff * vec3(texture(material.diffuse, texCoord)); // we use diffuseMap to replace the diffuse

    // spot light specular
    vec3 viewDir = lightDir; // (flash.position - FragPos) == (viewerPos - FragPos) == viewerPos
    vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specular = flash.specular * spec * vec3(texture(material.specular, texCoord)); // we also create a SpecularMap to replace specular

    // Make spotLight has a soft edge
    vec3 spotDir = normalize(flash.direction); // towards to Fragment
    float theta = dot(-lightDir, spotDir); // inner angle, both directions toward to fragments
    float epsilon = flash.cutOff - flash.outerCutOff; // 17.5 - 12.5 // outer angle - inner angle
    float intensity = clamp((theta - flash.outerCutOff)/epsilon,0.0f,1.0f); // we use two angles to control the Intensity AUTOMATICALLY
    vec3 diffAndSpe = (diffuse + specular) * intensity; // spotLight will only influence Diffuse and Specular
    
    // attenuation only affects spotLight
    float flashDistance = length(flash.position - FragPos);
    float attenuation = 1.0/(flash.constant + flash.linear * flashDistance + flash.quadratic * (flashDistance * flashDistance));;

    return (ambient + diffAndSpe) * attenuation;
}



void main()
{
    vec3 normal = normalize(Normal);
 
    // CAUTION: Functions' arguments can only pass by value (Pointer and Ref is Blocked!)
    vec3 directionalResult = DirectionalEffect(FragPos, normal, texCoord);
    // vec3 plightResult = pointLightEffect(light, FragPos, normal, texCoord);
    vec3 pointLightsResult = vec3(0.0);
    vec3 flash1Result = spotLightEffect(flash, FragPos, normal, texCoord);

    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        pointLightsResult += pointLightEffect(pointLights[i], FragPos, normal, texCoord);
    }

    vec3 emission = vec3(0.0f);
    if(texture(material.specular,texCoord).r == 0) 
    {
        emission = emissionControl * vec3(texture(material.emission, texCoord + vec2(0.0f, time * 0.5f)));
    }
    
    vec3 result = directionalResult + pointLightsResult + flash1Result + emission;
    result /= (result + vec3(1.0f)); // Normalize the final Result to [0,1]

    fragColor = vec4(result, 1.0f);

}
