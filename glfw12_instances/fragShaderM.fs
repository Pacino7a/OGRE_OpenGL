#version 330 core
// #define NR_POINT_LIGHTS 4
// #define NR_SPOT_LIGHTS 4

in VS_OUT
{
    // vec3 FragPos;
    // vec3 Normal;
    vec2 TexCoords;
    // mat3 TBN;

} fs_in;

out vec4 FragColor;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_specular1;
    sampler2D texture_specular2;
    sampler2D texture_normal1; // both can be Normals
    sampler2D texture_height1; //
    sampler2D texture_emission;
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

uniform Material material;

// Multiple Lights
// uniform Light pointLights[NR_POINT_LIGHTS];
// uniform FlashLight flashLights[NR_SPOT_LIGHTS];

// uniform Light light;
// uniform DirectionalLight dl;
// uniform FlashLight flash;
// uniform vec3 viewerPos;
// uniform vec3 emissionControl;
// uniform float time;

// vec3 DirectionalEffect(vec3 FragPos, vec3 normal, vec3 material_diffuse, vec3 material_specular)
// {
//     // Light is a DIRECTIONAL LIGHT!, not a point light
//     vec3 ambient = dl.ambient * material_diffuse; // Ambient

//     vec3 lightDir = normalize(-dl.direction); // Directional Light is in everywhere, so lightDir is nothing to do with FragPos.
//                                                   // but we expect it toward to Light Source, so we negate it
//     float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
//     vec3 diffuse = dl.diffuse * diff * material_diffuse; // we use diffuseMap to replace the diffuse

//     vec3 viewDir = normalize(viewerPos - FragPos);
//     vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
//     float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
//     vec3 specular = dl.specular * spec * material_specular; // we also create a SpecularMap to replace specular

//     return ambient + diffuse + specular;
// }

// vec3 pointLightEffect(const Light light, vec3 FragPos, vec3 normal, vec3 material_diffuse, vec3 material_specular)
// {
//     vec3 ambient = light.ambient * material_diffuse; // Ambient
    
//     vec3 lightDir = normalize( light.position - FragPos );
//     float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
//     vec3 diffuse = light.diffuse * diff * material_diffuse; // we use diffuseMap to replace the diffuse

//     vec3 viewDir = normalize(viewerPos - FragPos);
//     vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
//     float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
//     vec3 specular = light.specular * spec * material_specular; // we also create a SpecularMap to replace specular

//     // attenuation
//     float lightDistance = length(light.position - FragPos);
//     float attenuation = 1.0/(light.constant + light.linear * lightDistance + light.quadratic * (lightDistance * lightDistance));

//     return (ambient + diffuse + specular) * attenuation;
// }

// vec3 spotLightEffect(const FlashLight flash, vec3 FragPos, vec3 normal, vec3 material_diffuse, vec3 material_specular)
// {
//     vec3 ambient = flash.ambient * material_diffuse; 

//     // spot light diffuse
//     vec3 lightDir = normalize(flash.position - FragPos); // Dir towards to light source
//     float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
//     vec3 diffuse = flash.diffuse * diff * material_diffuse; // we use diffuseMap to replace the diffuse

//     // spot light specular
//     vec3 viewDir = lightDir; // (flash.position - FragPos) == (viewerPos - FragPos) == viewerPos
//     vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
//     float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
//     vec3 specular = flash.specular * spec * material_specular; // we also create a SpecularMap to replace specular

//     // Make spotLight has a soft edge
//     vec3 spotDir = normalize(flash.direction); // towards to Fragment
//     float theta = dot(-lightDir, spotDir); // inner angle, both directions toward to fragments
//     float epsilon = flash.cutOff - flash.outerCutOff; // 17.5 - 12.5 // outer angle - inner angle
//     float intensity = clamp((theta - flash.outerCutOff)/epsilon,0.0f,1.0f); // we use two angles to control the Intensity AUTOMATICALLY
//     vec3 diffAndSpe = (diffuse + specular) * intensity; // spotLight will only influence Diffuse and Specular
    
//     // attenuation only affects spotLight
//     float flashDistance = length(flash.position - FragPos);
//     float attenuation = 1.0/(flash.constant + flash.linear * flashDistance + flash.quadratic * (flashDistance * flashDistance));;

//     return (ambient + diffAndSpe) * attenuation;
// }



void main()
{
    vec4 base_diffuse = texture(material.texture_diffuse1,fs_in.TexCoords);
    vec4 overlay_diffuse = texture(material.texture_diffuse2,fs_in.TexCoords);
    vec4 mix_diffuse = mix(base_diffuse,overlay_diffuse,0.5f);
    vec3 material_diffuse = vec3(mix_diffuse);
    
    // vec4 base_specular = texture(material.texture_specular1,TexCoord);
    // vec4 overlay_specular = texture(material.texture_specular2,TexCoord);
    // vec4 mix_specular = mix(base_specular,overlay_specular,0.5f);
    // vec3 material_specular = vec3(mix_specular);

    // vec3 tangentNormal = texture(material.texture_height1,TexCoord).rgb; // maybe height
    // tangentNormal = tangentNormal * 2.0f - 1.0f; // [0,1] -> [-1,1] // standard decode
    // vec3 normal = normalize(TBN * tangentNormal); // Tangent Space -> World Space (Enhance Normal, Combined Normal and NormalTex)
    
    // normal = normalize(Normal); // Macro Normal Vector
 
    // CAUTION: Functions' arguments can only pass by value (Pointer and Ref is Blocked!)
    // vec3 directionalResult = DirectionalEffect(FragPos, normal, material_diffuse, material_specular);
    // vec3 singlePointResult = pointLightEffect(light, FragPos, normal, material_diffuse, material_specular);
    // vec3 pointLightsResult = vec3(0.0);
    // vec3 flash1Result = spotLightEffect(flash, FragPos, normal, material_diffuse, material_specular);

    // for(int i = 0; i < NR_POINT_LIGHTS; ++i)
    // {
    //     pointLightsResult += pointLightEffect(pointLights[i], FragPos, normal, texCoord, material_diffuse, material_specular);
    // }

    // vec3 emission = vec3(0.0f);
    // if(texture(material.specular,texCoord).r == 0) 
    // {
    //     emission = emissionControl * vec3(texture(material.emission, texCoord + vec2(0.0f, time * 0.5f)));
    // }
    
    // vec3 result = directionalResult + singlePointResult + flash1Result;
    // result /= (result + vec3(1.0f)); // Normalize the final Result to [0,1]


    FragColor = vec4(material_diffuse, 1.0f);

}
