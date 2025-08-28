#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform sampler2D texture1;
uniform samplerCube skybox;

void main()
{
    vec4 textureColor = texture(texture1, TexCoords);

    // Reflection
    vec3 I = normalize(FragPos - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    vec4 skyboxRef = vec4(texture(skybox,R).rgb, 1.0f); // complete reflection
    // Refraction
    float refr_ratio = 1.0f/1.52f;
    vec3 R_refraction = refract(I, normalize(Normal), refr_ratio);
    vec4 skyboxRefract = vec4(texture(skybox,R_refraction).rgb, 1.0f);
    // for the physically accurate, we can do the refraction again when the light out of the entity
    // but thickness is required
    vec3 sec_Refr = refract(normalize(R_refraction), normalize(Normal), 1.0f/refr_ratio);
    vec3 sampleDir = FragPos + sec_Refr * 5.0f; // thickness (5.0f)
    vec4 skyboxSecRefract = vec4(texture(skybox, sampleDir).rgb, 1.0f);

    vec3 mix = vec3(skyboxRef * textureColor); //  we can use a specular texture for multiplying this 
    mix = mix / (mix + vec3(1.0f));

    // FragColor = vec4(mix, 1.0f);
    FragColor = skyboxSecRefract;
    // if(gl_FragCoord.x <= 400)
    //     FragColor = vec4(1.0f,0.0f,0.0f,1.0f);
    // else
    //     FragColor = vec4(0.0f,0.0f,1.0f,1.0f);

}
