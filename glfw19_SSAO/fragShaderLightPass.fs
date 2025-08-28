#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

struct Light
{
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;
uniform Light light;

void main()
{   
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    
    // IN VIEW-SPACE!!
    // then calculate lighting as usual
    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    vec3 lighting  = ambient; // hard-coded ambient component
    // vec3 lighting  = vec3(0.0); // hard-coded ambient component
    vec3 viewDir  = normalize(-FragPos); // we are in View Space

    // diffuse
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = light.Color * spec * Specular;
    // attenuation
    float distance = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    // for(int i = 0; i < NR_LIGHTS; ++i)
    // {
    //     float distance = length(lights[i].Position - FragPos);
    //     if(distance < lights[i].Radius) // only do expensive lighting procedure when the fragment is in a Valid lighting Range
    //     {
    //         // diffuse
    //         vec3 lightDir = normalize(lights[i].Position - FragPos);
    //         vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
    //         // specular
    //         vec3 halfwayDir = normalize(lightDir + viewDir);  
    //         float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    //         vec3 specular = lights[i].Color * spec * Specular;
    //         // attenuation
    //         float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
    //         diffuse *= attenuation;
    //         specular *= attenuation;
    //         lighting += diffuse + specular;
    //     }

    // }

    FragColor = vec4(lighting, 1.0);
}
