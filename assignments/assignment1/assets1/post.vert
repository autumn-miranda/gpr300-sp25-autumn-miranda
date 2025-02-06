//assignment 1
#version 450
out vec2 UV;
out float depth;
out vec3 texPos;

vec4 vertices[3] = {
	vec4(-1, -1, 0, 0),
	vec4(3, -1, 2, 0),
	vec4(-1, 3, 0, 2)
};

void main()
{
	UV = vertices[gl_VertexID].zw;
    gl_Position = vec4(vertices[gl_VertexID].xyz, 1); 
	depth = vertices[gl_VertexID].z;
}  