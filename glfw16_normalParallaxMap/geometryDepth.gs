#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

out vec4 FragPos;

// one obj in scene can be lighted by multiple directions

uniform mat4 shadowMatrices[6]; // include view and projection

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            FragPos = gl_in[i].gl_Position; // worldPos
            gl_Position = shadowMatrices[face] * FragPos; // projection + view(different)  (The light interact with the Obj in many directions)
            EmitVertex();
        }
        EndPrimitive();
    }
}
