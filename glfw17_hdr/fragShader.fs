#version 330 core
// Multiple Render Targets (MRT)
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes // we pretend it a VEC4. we explicitly transform it into vec3, when we need to use it
};

// out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;

} fs_in;

struct Light
{
    vec3 Position;
    vec3 Color;
};

uniform sampler2D texture1;
uniform Light lights[16];

void main()
{
    // extract color and normal
    vec3 color = texture(texture1, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    // ambient
    vec3 ambient = 0.002 * color;

    vec3 viewDir = normalize(vec3(viewPos) - fs_in.FragPos);
    float maxDistance = 15.0;
    vec3 lighting = vec3(0.0);

    for(int i = 0; i < 16; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - fs_in.FragPos); // towards to the LightSource
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * color * lights[i].Color; // diff * (lightColr~vec3(1.0f)) * color(Tex)
        // reflection
        vec3 halfwayDir = normalize(lightDir + viewDir); // notice we didn't use reflect() function here
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0); // and here is Normal with HalfwayDir
        vec3 specular = lights[i].Color * spec;

        // compute attenuation (World Space)
        float distance = length(lights[i].Position - fs_in.FragPos);
        float attenuation;
        if(distance < maxDistance)
            attenuation = 1.0/(distance*distance); // without Gamma Correction, the denominator will ^2.2 when it shows on the monitor,
                                            // so linear attenuation is fine for this situation
                                            // otherwise, we correct the pixel intensity to LINEAR, we need a Quadratic attenuation
        else
            attenuation = 0.0;
        
        // apply attenuation
        diffuse *= attenuation;
        specular *= attenuation;

        // shadow
        // float shadow = ShadowCalculation(fs_in.FragPos, normal, lightDir);
        // mix up
        // vec3 lightingResult = ambient + (1.0 - shadow)*(diffuse + specular);

        lighting += diffuse;
    }
    
    lighting += ambient;

    // FragColor = vec4(vec3(closestDepth/far_plane));
    FragColor = vec4(lighting, 1.0); // notice we didn't do gamma correction(ENCODE) here ,
                                     // so the color value is LINEAR, you need to do gamma correction When you want to SHOW it!

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722)); // calculate the grayscale value to evalute the brightness
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
