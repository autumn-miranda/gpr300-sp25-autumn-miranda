//assignment 1
#version 450
out vec4 FragColor;
  
in vec2 UV;
in float depth;

#define resolution vec2(1080, 720);

uniform bool shadeBlur;
uniform bool sharpen;
uniform bool invert;
uniform bool outline;
uniform bool custom;
uniform float _Time;

uniform sampler2D _ColorBuffer;

#define sharpenKernel mat3(0, -1, 0, -1, 5, -1, 0, -1, 0)
#define outlineKernel mat3(0, -1, 0, -1, 4, -1, 0, -1, 0)
#define customKernel mat3(3, 2.5, 4, 0, 1, -10, 1.5, 0, -1)

void main()

{ 
	FragColor = texture(_ColorBuffer, UV);
	
	vec2 texelSize = 1.0 / textureSize(_ColorBuffer,0).xy;
	vec3 totalColor = vec3(0);

	if(shadeBlur){
		for(int y = -2; y <= 1; y++){
		   for(int x = -2; x <= 1; x++){
			  vec2 offset = vec2(x,y) * texelSize;
			  totalColor += texture(_ColorBuffer,UV + offset).rgb;
		   }
		}
		totalColor/=(4 * 4);//need to divide by 4x4 because I dont have a blur kernel
		FragColor = vec4(totalColor,1.0);
	}
	else if(sharpen)
	{
		for(int y = 0; y <= 2; y++){
			for(int x = 0; x <= 2; x++){
				vec2 offset = vec2(x,y) * texelSize;
				totalColor += texture(_ColorBuffer,UV + offset).rgb * sharpenKernel[x][y];
			}
		}
		//totalColor/=(3 * 3);
		FragColor = vec4(totalColor,1.0);
	}
	else if(outline)
	{
		for(int y = 0; y <= 2; y++){
			for(int x = 0; x <= 2; x++){
				vec2 offset = vec2(x,y) * texelSize;
				totalColor += texture(_ColorBuffer,UV + offset).rgb * outlineKernel[x][y];
			}
		}
		FragColor = vec4(totalColor,1.0);
	}
	
	if(custom)
	{
	//_time is seconds since window was initialized
		float d = mod(depth - _Time, 3.0) - 2.0;
		d = step(0.25, d);
		if(d==1){
			for(int y = 0; y <= 2; y++){
				for(int x = 0; x <= 2; x++){
					vec2 offset = vec2(x,y) * texelSize;
					totalColor += texture(_ColorBuffer,UV + offset).rgb * customKernel[x][y];
				}
			}
			FragColor = vec4(totalColor,1.0);
		}
	}

	if(invert){
	//commented code is how to set it without setting FragColor already
		//vec3 color = 1.0 - texture(_ColorBuffer, UV).rgb;
		FragColor = 1.0 - FragColor;//vec4(color, 1.0);
	}

}