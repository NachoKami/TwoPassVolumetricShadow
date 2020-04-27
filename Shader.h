/*Programmer: NekoKami2345
Date: 2/3/2020
*/
#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

class Shader
{
public:
	//Constructor
	Shader(const std::string& fileName);
	
	//Function to put the shader into use
	void Bind();

	//Function to destroy the Shader object
	virtual ~Shader();

	//Reference number for the shader program
	GLuint m_program;
protected:
private:
	//Only going to have two shaders
	static const unsigned int NUM_SHADERS = 2;
	void operator=(const Shader& other) {}

	void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage);
	GLuint CreateShader(const std::string& text, unsigned int type);
	std::string LoadShader(const std::string& fileName);

	
	GLuint m_shaders[NUM_SHADERS];
};

#endif
