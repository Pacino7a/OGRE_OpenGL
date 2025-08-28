#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 texCoord;

// in mat4 View;
// in mat4 Projection;

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

uniform vec3 viewerPos;
uniform Material material;
uniform Light light;
uniform vec3 emissionControl;
uniform float time;

uniform bool isXrayMode;
uniform bool isSecondPass;
uniform vec2 screenSize;
uniform float xRayRadius;

void main()
{
    float lightDistance = length(light.position - FragPos);
    float attenuation = 1.0/(light.constant + light.linear * lightDistance + light.quadratic * (lightDistance * lightDistance));
    vec3 normal = normalize(Normal);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord)); // Ambient

    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(normal,lightDir), 0.0f); // when the angle is bigger than pi/2, no light
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord)); // we use diffuseMap to replace the diffuse

    vec3 viewDir = normalize(viewerPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal); // this functions are available out of the box
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shiness * 128.0f); // 32 is the shiness, higher the shiness is the reflection happens on the entitiy more likely
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord)); // we also create a SpecularMap to replace specular

    // vec3 emission = emissionControl * vec3(texture(material.emission, texCoord));
    vec3 emission = vec3(0.0f);
    if(texture(material.specular,texCoord).r == 0) // check the box's border. if current fragment to render is not on the specular border,
    {                                              // we render the emission texture
        // emission = emissionControl * vec3(texture(material.emission, texCoord));                        // flow along y-axis by time
        emission = emissionControl * vec3(texture(material.emission, texCoord + vec2(0.0f, time * 0.5f))); // repeat when the coordnate over 1.0
    }

    // ambient + diffuse + specular and emission
    vec3 result = (ambient + diffuse + specular) * attenuation + emission;

    if(isXrayMode)
    {
        // world coord -> screen coord
        // vec4 clipPos = Projection * View * vec4(FragPos,1.0f);
        // vec3 ndc = clipPos.xyz/ clipPos.w;
        // vec2 screenCoord = (ndc.xy * 0.5f + 0.5) * screenSize;
        
        vec2 screenCoordE = gl_FragCoord.xy; // more effient way

        float dist = distance(screenCoordE, screenSize * 0.5f);
        bool inXrayArea = dist < xRayRadius;

        // false, true (first rendering, we discard the area in the circle)
        // true , false (second rendering, we discard the area that we have rendered in the first rendering)
        if(isSecondPass != inXrayArea)
            discard;
    }

    fragColor = vec4(result, 1.0f);

}
