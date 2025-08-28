#version 330 core

// in vec3 vertexColor;
in vec2 texCoord;


out vec4 fragColor;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    //fragColor = mix(texture(texture1, texCoord) ,texture(texture2, vec2(1-texCoord.x, texCoord.y)), 0.5f) * vec4(vertexColor, 1.0f);
    fragColor = mix(texture(texture1, texCoord) ,texture(texture2, vec2(1-texCoord.x, texCoord.y)), 0.5f);
}
