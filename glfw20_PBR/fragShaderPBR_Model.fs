// PBRt means Physically Based Rendering with Textures
#version 330 core
out vec4 FragColor;
layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
// uniform sampler2D aoMap;

uniform samplerCube irradianceMap; // Enhanced ambient (base ambient + diffuse and specular)
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT; 

uniform float ao;
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

const float PI = 3.14159265359;

// NDF
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NdotH = max(dot(N,H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = alpha2;
    float denom = (NdotH2 * (alpha2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Gsub
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// G
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N,V), 0.0);
    float NdotL = max(dot(N,L), 0.0);
    float ggx1 = geometrySchlickGGX(NdotV, roughness); // geometry occlusion
    float ggx2 = geometrySchlickGGX(NdotL, roughness); // geometry shadow

    return ggx1 * ggx2;
}

// F (kS has been included), get the ratio of Kd and Ks from the result of (Normal dot View)
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

vec3 getNormalFromNormalMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    // vec3 tangentNormal = texture(material.texture_height1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N  =  normalize(Normal);
    vec3 T  =  normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


void main()
{
    vec3 albedo     = texture(albedoMap, TexCoords).rgb;
    // vec3 albedo1 = texture(material.texture_diffuse1, TexCoords).rgb;
    // vec3 albedo2 = texture(material.texture_diffuse2, TexCoords).rgb;
    // vec4 mix_albedo = mix(albedo1, albedo2, 0.5f);
    // vec3 albedo = vec3(mix_albedo);

    vec3 normal     = getNormalFromNormalMap();

    float metallic  = texture(metallicMap, TexCoords).r;
    // float metallic1 = texture(material.texture_specular1, TexCoords).r;
    // float metallic2 = texture(material.texture_specular2, TexCoords).r;
    // float metallic = (metallic1 + metallic2) / 2.0 ;

    float roughness = texture(roughnessMap, TexCoords).r;
    // float roughness = texture(material.texture_roughness1, TexCoords).r;

    // float ao     = texture(aoMap, TexCoords).r;

    vec3 N = normal; // Normal as the direction of the rendering Obj's Surface
    vec3 V = normalize(vec3(viewPos) - FragPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04); // 4% specular reflection
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    for(int i = 0; i < 4; ++i)
    {
        vec3 L = normalize(lightPositions[i] - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - FragPos);
        float attenuation = 1.0 / (distance*distance);
        vec3 radiance = lightColors[i] * attenuation;

        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(clamp(dot(N,V), 0.0, 1.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N,V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal 
        // (pure metal shave no diffuse light).
        kD *= 1.0 - metallic;

        // Li -> the incident light scaling
        float NdotL = max(dot(N,L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // vec3 ambient = vec3(0.03) * albedo * ao; // from the light Source directly
    
    // irradiance ambient (diffuse), CONSIDERATE ENVIRONMENT LIGHTS (Global Lights)
    vec3 F = fresnelSchlickRoughness(max(dot(N,V),0.0), F0, roughness);
    // IBL WAY: DIFFUSE
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
   
    // IBL WAY: SPECULAR
    const float MAX_REFLECTION_LOD = 4.0; // 0 ~ 4 (5 levels)
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb; // Specular Color Extraction
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N,V), 0.0), roughness)).rg; // texCoords (viewer's angle, roughness), Specular HighLight Extraction
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    // MIX UP
    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0)); // HDR tone-Mapping
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);

}
