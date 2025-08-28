#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord;
layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes
};

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;

} vs_out;

uniform mat4 model;
uniform mat4 invModel;
uniform vec3 lightPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    vs_out.FragPos = vec3(model * vec4(aPos, 1.0f));
    vs_out.TexCoords = aTexCoord;

    mat4 normalMatrix = transpose(invModel);
    vec3 T = normalize(vec3(normalMatrix * vec4(aTangent,   0.0)));
    vec3 N = normalize(vec3(normalMatrix * vec4(aNormal,    0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    TBN = transpose(TBN); // you need use TBN's inverse(e.g. transpose here) if you want do calculate lights in Tangent Space
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * vec3(viewPos);// trans to vec3 because the align
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

}
