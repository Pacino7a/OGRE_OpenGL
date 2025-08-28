#version 330 core

out vec4 fragColor;

in vec2 TexCoord;

uniform sampler2D screenTexture1;

const float offset = 1.0/300.0; // if you want more mosaics, bring this down. Otherwise, more smooth

vec3 kernelizeColor()
{
    // offsets get the surrounding fragments of current fragment
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right  
    );

    // to maintain the Lightness, the sum of all the elements should be 1
    float narcotic_kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

    // blur makes the kernel goes too far from 1, so we need to normalize it by divide 16.
    float blur_kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16  
    );

    // edge detection
    float sharper_kernel[9] = float[](
        1,  1,  1,
        1, -8,  1,
        1,  1,  1
    );

    float test_kernel[9] = float[](
        2,  2,  2,
        2, -15,  2,
        2,  2,  2
    );

    vec3 sampleTex[9];
    for(int i=0; i < 9; ++i)
    {
        sampleTex[i] = vec3(texture(screenTexture1, TexCoord.st + offsets[i])); // collect surrounding fragments
    }
    
    vec3 col = vec3(0.0f);
    for(int i=0; i<9; ++i)
    {
        col += sampleTex[i] * test_kernel[i]; // apply the kernel on current fragment with its surrounding fragments
    }

    return col;
}

void main()
{

    vec3 kernelCol = kernelizeColor();
    fragColor = vec4(kernelCol, 1.0f);

    // vec3 col = texture(screenTexture1, TexCoord).rgb;
    // col = 1.0 - col; // reverse color
    // fragColor = vec4(col, 1.0);

    // float average = (col.r+col.g+col.b) / 3; // grayscale
    // float average = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b; // advanced Grayscale
    // fragColor = vec4(vec3(average),1.0f);
}
