#version 430 core

in vec3 TexCoords;
in vec3 FragPos;
in vec3 lightBufPos;

uniform vec3 userColor;
uniform bool tfs;

layout(binding=0) uniform sampler3D volume;
layout(binding=1) uniform sampler1D colorBar;
layout(binding=2) uniform sampler2D lightBuff;

uniform float s0, s1, s2, s3, s4, s5, s6, s7;

uniform int height, width;


out vec4 FragColor;
void main()
{
	if(tfs) {
		float intensity = texture(volume, TexCoords).r;
		if(intensity < .125)
		{
			if(s0 > 0)
				FragColor = vec4(texture(colorBar, (.02 + .95*(intensity))).rgb, s0);
			else
				discard;
		}
		else if(intensity < .25)
		{
			FragColor = vec4(texture(colorBar, (.02 + .95*(intensity))).rgb, s1);
		}
		else if(intensity < .375)
		{
			FragColor = vec4(texture(colorBar, (.02 + .95*(intensity))).rgb, s2);
		}
		else if(intensity < .5)
		{
			FragColor = vec4(texture(colorBar, (.02 + .95*(intensity))).rgb, s3);
		}
		else if(intensity < .625)
		{
			FragColor = vec4(texture(colorBar, (.02 + .95*(intensity))).rgb, s4);
		}
		else if(intensity < .75)
		{
			FragColor = vec4(texture(colorBar, (.02 + .95*(intensity))).rgb, s5);
		}
		else if(intensity < .875)
		{
			FragColor = vec4(texture(colorBar, (.02 + .95*(intensity))).rgb, s6);
		}
		else if(intensity <= 1)
		{
			FragColor = vec4(texture(colorBar, (.02 + .95*(intensity))).rgb, s7);
		}
		
		//FragColor = FragColor * (texture(lightBuff, vec2(xPos, yPos)));
		//FragColor = FragColor * (texture(lightBuff, vec2(0.5, 0.5)));
		FragColor = FragColor * texture(lightBuff, lightBufPos.xy);
		FragColor.xyz *= FragColor.a; 
		
		//FragColor = vec4(lightBufPos.x, lightBufPos.y, 0.0, 1.0);
	}
	else {
		//FragColor = vec4(userColor, 1.0);
		//FragColor = texture(lightBuff, vec2(.5, .5), 1.0);
		FragColor = vec4(FragPos, 1.0);
	}
}