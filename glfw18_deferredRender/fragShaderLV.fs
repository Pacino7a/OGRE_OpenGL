#version 330 core
out vec4 FragColor;
layout (std140) uniform Matrices
{
    mat4 view; // 4 * 16 Bytes
    mat4 projection; // 4 * 16 Bytes
    vec4 viewPos; // 16 Bytes
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light
{
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};

uniform Light light;

void main()
{   
    vec2 TexCoords = gl_FragCoord.xy / textureSize(gPosition, 0);
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // then calculate lighting as usual
    vec3 lighting  = vec3(0.0); // hard-coded ambient component
    vec3 viewDir  = normalize(vec3(viewPos) - FragPos);

    // diffuse
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = light.Color * spec * Specular;
    // attenuation
    float distance = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    // ========= 羽化边界的核心代码 ==========
    
    // 1. 计算像素到光源中心的距离
    float distanceToFragment = length(light.Position - FragPos);
    
    // 2. 定义羽化开始的位置，例如从半径的 80% 处开始
    float featherStart = light.Radius * 0.8; 
    
    // 3. 使用 smoothstep 创建一个从 1 到 0 的平滑因子
    // 当 distanceToFragment < featherStart 时, 结果为 1 (完全亮)
    // 当 distanceToFragment > light.Radius 时, 结果为 0 (完全暗)
    // 在两者之间时，平滑过渡
    float featherFactor = 1.0 - smoothstep(featherStart, light.Radius, distanceToFragment);

    FragColor = vec4(lighting * featherFactor, 1.0);

}
