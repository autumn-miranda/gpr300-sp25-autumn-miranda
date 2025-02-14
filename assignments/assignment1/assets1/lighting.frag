//assignment 2
#version 450

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 _EyePos;
//light pointing straight down
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);//white light

uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

struct Material{
	float Ka; // ambient coefficient (0-1)
	float Kd; // diffuse coefficient (0-1)
	float Ks; // specular coefficient (0-1)
	float Shininess; //affects size of specular highlight
};
uniform Material _Material;

float ShadowCalculation(vec4 fragPosLightSpace)
{
//perspective divide. Unnecessary for orthographic
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(fs_in.Normal, _LightDirection)), 0.005);


	float shadow = (currentDepth - bias > closestDepth)? 1.0: 0.0;

	return shadow;
}

void main()
{
	//make sure fragment normal is still lenght 1 after interpolation
	vec3 normal = normalize(fs_in.Normal);
	//light pointing straight down
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal, toLight),0.0);
	//direction towards eye
	vec3 toEye = normalize(_EyePos - fs_in.FragPos);
	//blinn-phong uses half angle
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, h),0.0), _Material.Shininess);
	//combination of specular and diffuse reflection
	vec3 dsColor = _LightColor * (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor);
	//add some ambient light
	vec3 ambient = _AmbientColor * _Material.Ka;
	vec3 objectColor = texture(diffuseTexture, fs_in.TexCoords).rgb;
	
	/*vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(1.0);
	vec3 ambient = 0.15 * lightColor;
	
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;*/

	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
	vec3 lighting = (ambient + (1.0 - shadow) * dsColor) * objectColor;
	//vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

	FragColor = vec4(lighting, 1.0);
}