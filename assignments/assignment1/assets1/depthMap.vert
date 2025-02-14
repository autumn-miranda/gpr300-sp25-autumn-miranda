//assignment 2
#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 lightSpaceMatrix;
uniform mat4 _Model;

void main()
{
	//TexCoords = aTexCoords;
	gl_Position = lightSpaceMatrix * _Model * vec4(aPos, 1.0);
}