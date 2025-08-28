#version 330 core
out vec4 FragColor;

layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes // we pretend it a VEC4. we explicitly transform it into vec3, when we need to use it
    bool blinn; // 4
    bool gamma; // 4
};

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec4 FragPosLightSpace;

} fs_in;

uniform sampler2D texture1;
uniform sampler2D shadowMap;
uniform vec3 lightPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // perform perspective divide (Clip Space -> NDC space)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // [-1,1]
    projCoords = projCoords * 0.5 + 0.5; // Z in [0,1]

    float closestDepth = texture(shadowMap, projCoords.xy).r; // the closestDepth's maximum is 1.0 (which means Surface)
    float currentDepth = projCoords.z;

    if(currentDepth > 1.0)
        return 0.0;
    
    // float bias = max(0.007 * (1.0 - dot(normal, lightDir)), 0.005);  // Most of the time it's simply a matter of slowly incrementing the bias until all acne is removed.
    float bias = max(0.006 * (1.0 - dot(normal, lightDir)), 0.005);
    // bias = 0.0;
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  // solve fake cover

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    return shadow;
}

void main()
{
    // texture color
    vec3 color = texture(texture1, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.002 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos); // towards to the LightSource
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color; // diff * (lightColr~vec3(1.0f)) * color(Tex)
    // reflect
    vec3 viewDir = normalize(vec3(viewPos) - fs_in.FragPos);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir); // notice we didn't use reflect() function here
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0); // and here is Normal with HalfwayDir
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    vec3 specular = vec3(0.3f) * spec;

    // compute attenuation
    float maxDistance = 15.0;
    float distance = length(lightPos - fs_in.FragPos);
    float attenuation;
    if(distance < maxDistance)
        attenuation = 1.0/(gamma? distance*distance : distance); // without Gamma Correction, the denominator will ^2.2 when it shows on the monitor,
                                        // so linear attenuation is fine for this situation
                                        // otherwise, we correct the pixel intensity to LINEAR, we need a Quadratic attenuation
    else
        attenuation = 0.0;
    // apply attenuation
    diffuse *= attenuation;
    specular *= attenuation;

    // shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
    // mix up
    vec3 lightingResult = ambient + (1.0 - shadow)*(diffuse + specular);

    // gamma correction
    if(gamma)
    {
        float gammaValue = 2.2;
        lightingResult = pow(lightingResult, vec3(1.0/gammaValue));// Make the pixel intensity curve convex
    }

    FragColor = vec4(lightingResult, 1.0);
}
