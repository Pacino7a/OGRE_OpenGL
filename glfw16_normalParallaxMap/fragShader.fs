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
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;

} fs_in;

uniform sampler2D texture1;
uniform sampler2D normalMap;
uniform sampler2D displacementMap;
uniform samplerCube shadowMap;

uniform vec3 lightPos;
uniform float far_plane;
uniform float height_scale;

float ShadowCalculation(vec3 fragPos, vec3 normal, vec3 lightDir)
{
    // World Space

    vec3 fragToLight = fragPos - lightPos;
    // float closestDepth = texture(shadowMap, fragToLight).r; // relative depth here (0 ~ 1)
    // closestDepth *= far_plane;

    float currentDepth = length(fragToLight);

    // Traditional Shadow with Bias
    // float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); // this can be done in tangent space
    // bias = 0.005;
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0; 

    // PCF (Percentage-Closer Filtering)
    // float shadow  = 0.0;
    // float bias    = 0.15; 
    // float samples = 4.0;
    // float offset  = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5)) //x4
    // {
    //     for(float y = -offset; y < offset; y += offset / (samples * 0.5))
    //     {
    //         for(float z = -offset; z < offset; z += offset / (samples * 0.5))
    //         {
    //             float closestDepth = texture(shadowMap, fragToLight + vec3(x, y, z)).r; 
    //             closestDepth *= far_plane;   // undo mapping [0;1]
    //             if(currentDepth - bias > closestDepth)
    //                 shadow += 1.0;
    //         }
    //     }
    // }
    // shadow /= (samples * samples * samples);
    
    // Advanced PCF
    vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );


    float shadow = 0.0;
    float bias   = 0.08;
    int samples  = 20;
    float viewDistance = length(vec3(viewPos) - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);

    return shadow;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * height_scale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(displacementMap, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(displacementMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth; // negative
    float beforeDepth = texture(displacementMap, prevTexCoords).r - currentLayerDepth + layerDepth; // positive
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth); // positive
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight); // when the gap of after Depth is bigger -> weight is high -> the real P is close to prevTexCoords

    return finalTexCoords;
}

// vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
// {
//     // Native Ver.
//     // float height = texture(displacementMap, texCoords).r;
//     // vec2 p = viewDir.xy / viewDir.z * (height_scale * height);
//     // return texCoords - p;

//     // multiple sample layers Ver.
//     const float minimumLayers = 8.0;
//     const float maximumLayers = 32.0;
//     float numLayers = mix(maximumLayers, minimumLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
//     // float numLayers = 1000;
//     float layerDepth = 1.0 / numLayers;
//     vec2 p = viewDir.xy / viewDir.z * height_scale; // ready the Dir Vec (NO divide z!)
//     vec2 deltaTexOffset = p / numLayers;
//     vec2 newTexCoords = texCoords;
//     float currentDepthMapValue = texture(displacementMap, texCoords).r;
//     float currentLayerDepthValue = 0.0;
    
//     // When the DepthMap's value is bigger, the smaller layer depth value is The most suitable LENGTH of vector p
//     for(;currentDepthMapValue > currentLayerDepthValue;)
//     {
//         newTexCoords -= deltaTexOffset; // shift the texcoords along the direction of P
//         currentDepthMapValue = texture(displacementMap, newTexCoords).r; // extract next depth value on the depth Texture(not depthMap -- depth test kind)
//         currentLayerDepthValue += layerDepth; // shift to the next layer value for comparing
//     }
    
//     vec2 prevTexCoords = newTexCoords + deltaTexOffset;
    
//     float currentGap = currentDepthMapValue - currentLayerDepthValue;
//     float prevGap = texture(displacementMap, prevTexCoords).r - currentLayerDepthValue + layerDepth;

//     float weight = currentGap / (currentGap + prevGap); // Bigger the currentGap is(far from current), the finalTexCoords is more based on the prevCoords
//     vec2 finalTexCoords = prevTexCoords * weight + newTexCoords * (1 - weight);

//     return finalTexCoords;
// }

void main()
{
    // to use displacementMap, we need overwrite the Texcoords
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 TexCoords = ParallaxMapping(fs_in.TexCoords, viewDir); // Tangent Space
    if(TexCoords.x > 1.0 || TexCoords.y > 1.0 || TexCoords.x < 0.0 || TexCoords.y < 0.0)
        discard;
    // extract color and normal
    vec3 color = texture(texture1, TexCoords).rgb;
    vec3 normal = texture(normalMap, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    // ambient
    vec3 ambient = 0.002 * color;

    // we'll calculate the diff and spec in the Tangent Space!!
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos); // towards to the LightSource
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color; // diff * (lightColr~vec3(1.0f)) * color(Tex)
    // reflect
    vec3 halfwayDir = normalize(lightDir + viewDir); // notice we didn't use reflect() function here
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0); // and here is Normal with HalfwayDir
    vec3 specular = vec3(0.3f) * spec;

    // compute attenuation (World Space)
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
    float shadow = ShadowCalculation(fs_in.FragPos, normal, lightDir);
    // mix up
    // vec3 lightingResult = ambient + (1.0 - shadow)*(diffuse + specular);

    vec3 lightingResult = ambient + diffuse + specular;

    // gamma correction
    float gammaValue = 2.2;
    lightingResult = pow(lightingResult, vec3(1.0/gammaValue));// Make the pixel intensity curve convex

    // FragColor = vec4(vec3(closestDepth/far_plane));
    FragColor = vec4(lightingResult, 1.0);
}
