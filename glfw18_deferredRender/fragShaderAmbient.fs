#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gAlbedoSpec; // 我们只需要 Albedo 贴图

// 全局环境光颜色
uniform vec3 ambientColor;

void main()
{
    // 获取物体的漫反射颜色
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;
    
    // 最终的环境光贡献就是 Albedo * 环境光颜色
    FragColor = vec4(albedo * ambientColor, 1.0);
}
