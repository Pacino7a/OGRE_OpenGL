#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexDirs;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // make the skybox's depth value always be the maximum Value of the depth (i.e 1.0f)
    TexDirs = aPos;

}
