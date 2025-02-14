//assignment 2
#version 450
out vec4 FragColor;

out vec2 TexCoords;

uniform sampler2D depthMap;

void main()
{
	gl_FragDepth = gl_FragCoord.z;// this is what happens behind the scene

	//render depthmap to a quad
	float depthValue = texture(depthMap, TexCoords).r;
	FragColor = vec4(vec3(depthValue), 1.0);//orthographic*/
}