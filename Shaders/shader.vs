#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;
uniform mat4 lightMVP;
uniform mat4 viewMat;
uniform mat4 projMat;

uniform mat4 lightView;
uniform mat4 lightModel;
uniform mat4 lightProj;

smooth out vec3 TexCoords;
out vec3 FragPos;
out vec3 lightBufPos;
void main()
{
	gl_Position = MVP * vec4(aPos, 1.0);
	vec4 tempBuf = lightProj * lightView * lightModel * vec4(aPos, 1.0);
	lightBufPos = vec3(tempBuf.x/tempBuf.w, tempBuf.y/tempBuf.w, tempBuf.z/tempBuf.w);
	lightBufPos = (lightBufPos + (1.0, 1.0, 1.0))/2;
	FragPos = aPos;
	TexCoords = (1.0, 1.0, 1.0) - aPos;
}