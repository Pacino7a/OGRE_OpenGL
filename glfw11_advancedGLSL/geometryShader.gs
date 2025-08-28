# version 330
// layout (points) in; // Primitive from vertexShader
// layout (points, max_vertices = 1) out; // Modify the Primitive here and Output this kind of Primitive
layout (triangles) in;
// layout (triangle_strip, max_vertices = 5) out;
layout (line_strip, max_vertices = 2) out;

const float MAGNITUDE = 0.4;

in VS_OUT
{
    // vec2 TexCoords;
    vec3 Normal;
    mat4 Projection;
} gs_in[];

// out vec3 gColor;
// out vec2 TexCoords;

// uniform float time;

// void build_house(vec4 position)
// {
//     gColor = gs_in[0].vertexColor;
//     gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:bottom-left
//     EmitVertex();   
//     gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:bottom-right
//     EmitVertex();
//     gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:top-left
//     EmitVertex();
//     gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:top-right
//     EmitVertex();
//     gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:top
//     gColor = vec3(1.0f, 1.0f, 1.0f);
//     EmitVertex();
//     EndPrimitive();
// }

// vec3 getNormal()
// {
//     vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
//     vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
//     return normalize(cross(a, b));
// }
// 
// vec4 explode(vec4 position, vec3 normal)
// {
//     float magnitude = 2.0f;
//     vec3 direction = normal * ((sin(time) + 1.0)/ 2.0) * magnitude;
//     return position + vec4(direction, 1.0f);
// }

// Notice that we need do projection in this shader, instead of the vertexShader
// Because if you do projection, the vertex will lose the real spatial semantics
// the line_strip that you want to draw will be a MESS

void GenerateLine(int index) 
{
    gl_Position = gs_in[index].Projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = gs_in[index].Projection * (gl_in[index].gl_Position + 
                                vec4(gs_in[index].Normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    // Notice that gl_in[] is an array, because the input of this shader is a primitive,
    // instead of a single vertex.
    // A Primitive contains Many vertices, but we need to handle data in Vertex level,
    // so we handle the (vertices) array.

    // Pass through Test
    // gl_Position = gl_in[0].gl_Position;
    // EmitVertex(); // Submit current new Position or Something else to the Output primitive (make the vertex a part of it)
    // EndPrimitive(); // Primitive Construction Done (we recive 1 emitted vertex for building the primitive in current Case)

    // build_house(gl_in[0].gl_Position);

    // vec3 normal = getNormal();
    // gl_Position = explode(gl_in[0].gl_Position, normal);
    // TexCoords = gs_in[0].TexCoords;
    // EmitVertex();
    // gl_Position = explode(gl_in[1].gl_Position, normal);
    // TexCoords = gs_in[1].TexCoords;
    // EmitVertex();
    // gl_Position = explode(gl_in[2].gl_Position, normal);
    // TexCoords = gs_in[2].TexCoords;
    // EmitVertex();
    // EndPrimitive();

    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}
