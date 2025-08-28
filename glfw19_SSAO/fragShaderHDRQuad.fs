#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrTex;
uniform float exposure;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrTex, TexCoords).rgb; // to get a HDR result, you not only need a HDR Color,
                                                    // but also you need a Tone-Mapping, to project the HDR values to the Right LSR or SDR Space
    
    // vec3 blurColor = texture(bloomBlur, TexCoords).rgb;
    // hdrColor += blurColor; // additive Blending

    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0/gamma)); // LINEAR COLOR -> PERCEPTUAL LINEAR
    
    FragColor = vec4(result, 1.0);
}
