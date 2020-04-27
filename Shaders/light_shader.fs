#version 430 core

in vec3 TexCoords;
in vec3 FragPos;

//uniform vec3 userColor;
//uniform bool tfs;

layout(binding=0) uniform sampler3D volume;
//layout(binding=1) uniform sampler1D colorBar;
layout(binding=2) uniform sampler2D lightBuff;

uniform float s0, s1, s2, s3, s4, s5, s6, s7;

out vec4 FragColor;
void main()
{
	FragColor = vec4(0.0, 0.0, 0.0, 0.1);
	float intensity = texture(volume, TexCoords).r;
		if(intensity < .125)
		{
			if(s0 > 0)
				FragColor = vec4(0.0, 0.0, 0.0, s0);
			else
				discard;
		}
		else if(intensity < .25)
		{
			if(s1 > 0)
				FragColor = vec4(0.0, 0.0, 0.0, s1);
			else
				discard;
		}
		else if(intensity < .375)
		{
			if(s2 > 0)
				FragColor = vec4(0.0, 0.0, 0.0, s2);
			else
				discard;
		}
		else if(intensity < .5)
		{
			if(s3 > 0)
				FragColor = vec4(0.0, 0.0, 0.0, s3);
			else
				discard;
		}
		else if(intensity < .625)
		{
			if(s4 > 0)
				FragColor = vec4(0.0, 0.0, 0.0, s4);
			else
				discard;
		}
		else if(intensity < .75)
		{
			if(s5 > 0)
				FragColor = vec4(0.0, 0.0, 0.0, s5);
			else
				discard;
		}
		else if(intensity < .875)
		{
			if(s6 > 0)
				FragColor = vec4(0.0, 0.0, 0.0, s6);
			else
				discard;
		}
		else if(intensity <= 1)
		{
			if(s7 > 0)
				FragColor = vec4(0.0, 0.0, 0.0, s7);
			else
				discard;
		}
		FragColor.xyz *= FragColor.a;
}