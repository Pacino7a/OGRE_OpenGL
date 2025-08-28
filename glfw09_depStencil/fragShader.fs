#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;


out vec4 fragColor;

uniform sampler2D texture1;
uniform bool drawFrame;
uniform bool drawGrass;

// uniform float near;
// uniform float far;

// float LinearizeDepth(float depth) 
// {
//     float z = depth * 2.0 - 1.0; // back to NDC 
//     return (2.0 * near * far) / (far + near - z * (far - near));
// }

void main()
{
    // fragColor = vec4(vec3(LinearizeDepth(gl_FragCoord.z)/far),1.0f);

    if(drawFrame)
        fragColor = vec4(0.04, 0.28, 0.26, 1.0);
    else if (drawGrass)
    {
        vec4 textureColor = texture(texture1,TexCoord);
        if(textureColor.a < 0.05) // if transparency is low (Full transparent)
            discard; // discard this fragment, return
        fragColor = textureColor;
    }
    else
        fragColor = texture(texture1, TexCoord);


}
