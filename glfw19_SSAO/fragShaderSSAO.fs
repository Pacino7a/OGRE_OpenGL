#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;

// tile noise texture over screen, based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); // screen = 800x600

void main()
{
    // All of those Operate in View Space
    vec3 fragPos   = texture(gPosition, TexCoords).xyz;
    vec3 normal    = texture(gNormal, TexCoords).rgb;
    vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal); 

    // AO control Panel
    float occlusion = 0.0;
    int kernelSize = 32; // bigger is stronger
    float radius = 0.5; // bigger
    float bias = 0.015; // smaller
    float power = 4.0; // bigger
    vec3 samplePos;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample positionsamples
        samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        vec4 offset = vec4(samplePos, 1.0);
        offset      = projection * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 ~ 1.0  

        float sampleDepth = texture(gPosition, offset.xy).z;
        // occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;

    }

    occlusion = 1.0 - (occlusion / kernelSize);
    occlusion = pow(occlusion, power);
    FragColor = occlusion;
}
