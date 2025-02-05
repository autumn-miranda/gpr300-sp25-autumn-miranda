//assignment 0
#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Vertex position in model space
layout(location = 1) in vec3 vNormal; //Vertex position in model space
layout(location = 2) in vec2 vTexCoord;//vertex texture coord UV


uniform mat4 _Model; //Model->World Matrix
uniform mat4 _ViewProjection; //Combined View->Projection Matrix

out Surface
{
	vec3 WorldPos;//position in world space
	vec3 WorldNormal; //Output to next shader
	vec2 TexCoord;
}vs_out;

void main(){
	//transform vertex position to world space
	vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));
	//transform vertex normal to world space using normal matrix
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTexCoord;
	//Transform vertex position to homogeneous clip space
	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}
