#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DMS screenTextureMS; // multisampled texture
uniform ivec2 viewportSize; // resolution in pixels
uniform int samples; // 4 or 8 etc.

void main()
{
    ivec2 texelCoord = ivec2(TexCoords * vec2(viewportSize));

    // Average AA
    // vec4 color = vec4(0.0);
    // for (int i = 0; i < samples; ++i)
    // {
    //     color += texelFetch(screenTextureMS, texelCoord, i);
    // }
    // color /= float(samples);

    // weighted Average AA
    float weights[4] = float[](0.4, 0.3, 0.2, 0.1);
    vec4 color = vec4(0.0);
    for (int i = 0; i < 4; ++i) 
    {
        color += texelFetch(screenTextureMS, texelCoord, i) * weights[i];
    }

    FragColor = color;
}
