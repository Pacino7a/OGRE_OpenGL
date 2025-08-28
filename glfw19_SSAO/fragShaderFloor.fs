#version 330 core
layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes // we pretend it a VEC4. we explicitly transform it into vec3, when we need to use it
};

out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos; // worldPos
    vec2 TexCoords;
    vec3 Normal;

} fs_in;

uniform sampler2D texture1;
// uniform samplerCube shadowMap; 
uniform vec3 lightPos;
// uniform float far_plane;

// float ShadowCalculation(vec3 fragPos, vec3 normal, vec3 lightDir)
// {
//     vec3 fragToLight = fragPos - lightPos;
//     // float closestDepth = texture(shadowMap, fragToLight).r; // relative depth here (0 ~ 1)
//     // closestDepth *= far_plane;

//     float currentDepth = length(fragToLight);

//     // Traditional Shadow with Bias
//     // float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
//     // bias = 0.005;
//     // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0; 

//     // PCF (Percentage-Closer Filtering)
//     // float shadow  = 0.0;
//     // float bias    = 0.15; 
//     // float samples = 4.0;
//     // float offset  = 0.1;
//     // for(float x = -offset; x < offset; x += offset / (samples * 0.5)) //x4
//     // {
//     //     for(float y = -offset; y < offset; y += offset / (samples * 0.5))
//     //     {
//     //         for(float z = -offset; z < offset; z += offset / (samples * 0.5))
//     //         {
//     //             float closestDepth = texture(shadowMap, fragToLight + vec3(x, y, z)).r; 
//     //             closestDepth *= far_plane;   // undo mapping [0;1]
//     //             if(currentDepth - bias > closestDepth)
//     //                 shadow += 1.0;
//     //         }
//     //     }
//     // }
//     // shadow /= (samples * samples * samples);
    
//     // Advanced PCF
//     vec3 sampleOffsetDirections[20] = vec3[]
//     (
//         vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
//         vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
//         vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
//         vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
//         vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
//     );


//     float shadow = 0.0;
//     float bias   = 0.08;
//     int samples  = 20;
//     float viewDistance = length(vec3(viewPos) - fragPos);
//     float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
//     for(int i = 0; i < samples; ++i)
//     {
//         float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
//         closestDepth *= far_plane;   // undo mapping [0;1]
//         if(currentDepth - bias > closestDepth)
//             shadow += 1.0;
//     }
//     shadow /= float(samples);

//     return shadow;
// }

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
    vec3 halfwayDir = normalize(lightDir + viewDir); // notice we didn't use reflect() function here
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0); // and here is Normal with HalfwayDir
    vec3 specular = vec3(0.3f) * spec;

    // compute attenuation
    float maxDistance = 15.0;
    float distance = length(lightPos - fs_in.FragPos);
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
    vec3 lightingResult = ambient + (diffuse + specular);
    // vec3 lightingResult = ambient + (1.0 - shadow)*(diffuse + specular);

    // gamma correction
    float gammaValue = 2.2;
    lightingResult = pow(lightingResult, vec3(1.0/gammaValue));// Make the pixel intensity curve convex

    // FragColor = vec4(vec3(closestDepth/far_plane));
    FragColor = vec4(lightingResult, 1.0);
}
