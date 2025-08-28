#version 330 core

// in vec3 gColor;
in vec2 TexCoords;

out vec4 FragColor;

struct Material // for Rendering Assimp Model 
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

uniform Material material;

void main()
{
    // FragColor = vec4(gColor, 1.0);
    FragColor = texture(material.texture_diffuse1, TexCoords);

}
