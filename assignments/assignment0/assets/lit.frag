#version 450
out vec4 FragColor; //The color of this fragment

in Surface
{
	vec3 WorldPos;
	vec3 WorldNormal; //Interpolated of this fragment 
	vec2 TexCoord;
}fs_in;

uniform sampler2D _MainTex;//2D texture sampler
uniform vec3 _EyePos;
//light pointing straight down
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);//white light

uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

void main(){
	//make sure fragment normal is still lenght 1 after interpolation
	vec3 normal = normalize(fs_in.WorldNormal);
	//light pointing straight down
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal, toLight),0.0);
	//direction towards eye
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	//blinn-phong uses half angle
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, h),0.0), 128);
	//combination of specular and diffuse reflection
	vec3 lightColor = _LightColor * (diffuseFactor + specularFactor);
	//add some ambient light
	lightColor += _AmbientColor;
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	FragColor = vec4(objectColor * lightColor, 1.0);
}


//Blinn-Phong reflectance model