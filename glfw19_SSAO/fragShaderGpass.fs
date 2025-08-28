#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_specular1;
    sampler2D texture_specular2;
    sampler2D texture_normal1; // 
    sampler2D texture_height1; // Normals
    sampler2D texture_emission;
    float shiness;
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform Material material;
uniform bool manualSpec;
uniform float specular;

void main()
{    
    // Notice we are in View Space!

    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse color per-fragment
    gAlbedoSpec.rgb = texture(material.texture_diffuse1, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    if(!manualSpec)
        gAlbedoSpec.a = texture(material.texture_specular1, TexCoords).r;
    else
        gAlbedoSpec.a = specular;

    // gAlbedoSpec.rgb = vec3(1.0);
    // gAlbedoSpec.a = 0.2;
}
