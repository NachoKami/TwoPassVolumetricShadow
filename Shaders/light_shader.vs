#version 430 core
layout (location = 0) in vec3 aPos;


uniform mat4 MVP;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 TexCoords;
out vec3 FragPos;
void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = gl_Position.xyz;
	TexCoords = vec3(1.0, 1.0, 1.0) - aPos;
}