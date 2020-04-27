/*	Programmer: NekoKami2345
	Date: 4/5/2020
	Implementing basic shadows in a volumetrically rendered scene
	by using a two-pass rendering algorithm
*/

//LIBRARY INCLUDES
//GLEW
#define GLEW_STATIC
#include <GL/glew.h>
//GLFW
#include <GLFW/glfw3.h>
//NANOGUI
#include "nanogui/nanogui.h"
//GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
//STBI
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//STANDARD INCLUDES
#include <stdio.h>
#include <iostream>

//CLASS INCLUDES
#include "Shader.h"
#include "Camera.h"

//NAMESPACES
using namespace nanogui;

//STRUCTS
struct DirectionalLight {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	Color color;
	DirectionalLight() : position(0.5f, 0.5f, -3.0f), front(0.0f, 0.0f, 1.0f),
		right(-1.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f), color(1.0f, 1.0f, 1.0f, 1.0f) {}
	void rotateLightX(float angle) {
		glm::mat4 rotation_mat(1.0f);
		rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), right);
		up = glm::normalize(glm::vec3(rotation_mat * glm::vec4(up, 1.0f)));
		front = glm::normalize(glm::cross(up, right));
	}
	void rotateLightY(float angle) {
		glm::mat4 rotation_mat(1.0f);
		rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), up);
		front = glm::normalize(glm::vec3(rotation_mat * glm::vec4(front, 1.0f)));
		right = glm::normalize(glm::cross(front, up));
	}
	void rotateLightZ(float angle) {
		glm::mat4 rotation_mat(1.0f);
		rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), front);
		right = glm::normalize(glm::vec3(rotation_mat * glm::vec4(right, 1.0f)));
		up = glm::normalize(glm::cross(right, front));
	}
	void reset() {
		front = glm::vec3(0.0f, 0.0f, 1.0f);
		right = glm::vec3(-1.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
	}
};

//ENUMERATIONS
enum modelName {
	HEAD = 0,
	TEAPOT,
	BUCKY,
	BONSAI
};
enum renderType {
	LINE = 0,
	TRIANGLE,
	POINT
};

//VARIABLES
//Object init
Camera camera = Camera(glm::vec3(0.5f, 0.5f, 3.0f));
//Window variables
int width = 1860, height = 900;

//Rotate value(amount to rotate by)
int rotateVal = 10;
//Model being loaded in
modelName model = TEAPOT;
renderType render = LINE;
std::string colorPath = "data_and_code/colorbar.png";
//Render color
Color objColor;
//bool to determine transfer function sign
bool transSign = false;
//int for sample rate
int sampleRate = 10;
//LIGHT VARIABLES
DirectionalLight light;
int lightRotateVal = 10;
//Width and height of the light buffer
int lightWidth = 300;
int lightHeight = 300;
//Bool to determine if slices are being rendered backToFront for the camera or not
bool backToFront = true;

//Variables fo use in the nanogui graph, and for use in the transfer function
float s0 = 0.0f, s1 = 0.0f, s2 = 0.0f, s3 = 0.0f, s4 = 0.0f, s5 = 0.0f, s6 = 0.0f, s7 = 0.0f, s8 = 0.0f;

//vars for determining which vertex is closest
float closeZ;
float farZ;
//var for walking through the planes
float zSeperation;

//Nanogui variables
Screen* screen;

//OpenGL variables
unsigned int tex3D;
unsigned int colorBarID;

//Light pointer data
glm::vec3 pointer_vertices[12] =
{
	glm::vec3(0.0, 0.1, -0.3),
	glm::vec3(-0.1, -0.1, -0.3),
	glm::vec3(0.1, -0.1, -0.3),

	glm::vec3(0.1, -0.1, -0.3),
	glm::vec3(0.0, 0.0, 0.1),
	glm::vec3(0.0, 0.1, -0.3),

	glm::vec3(0.0, 0.0, 0.1),
	glm::vec3(-0.1, -0.1, -0.3),
	glm::vec3(0.0, 0.1, -0.3),

	glm::vec3(-0.1, -0.1, -0.3),
	glm::vec3(0.0, 0.0, 0.1),
	glm::vec3(0.1, -0.1, -0.3)
};

//Bounding cube data
GLfloat cube_vertices[24] = {
	0.0, 0.0, 0.0,
	0.0, 0.0, 1.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 1.0,
	1.0, 0.0, 0.0,
	1.0, 0.0, 1.0,
	1.0, 1.0, 0.0,
	1.0, 1.0, 1.0
};

glm::vec3 cube_verts[8] = {
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(1.0f, 0.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 0.0f),
	glm::vec3(1.0f, 1.0f, 1.0f)
};

glm::vec3 test_cube_verts[8] = {
	glm::vec3(-0.5f,-0.5f,-0.5f),
	glm::vec3(-0.5f,-0.5f,0.5f),
	glm::vec3(-0.5f, 0.5f,-0.5f),
	glm::vec3(-0.5f, 0.5f,0.5f),
	glm::vec3(0.5f,-0.5f, -0.5f),
	glm::vec3(0.5f,-0.5f, 0.5f),
	glm::vec3(0.5f, 0.5f, -0.5f),
	glm::vec3(0.5f, 0.5f, 0.5f)
};

GLuint cube_indices[36] = {
	1,5,7,
	7,3,1,
	0,2,6,
	6,4,0,
	0,1,3,
	3,2,0,
	7,5,4,
	4,6,7,
	2,3,7,
	7,6,2,
	1,0,4,
	4,5,1
};

GLuint cube_edges[24]{
	1,5,
	5,7,
	7,3,
	3,1,
	0,4,
	4,6,
	6,2,
	2,0,
	0,1,
	2,3,
	4,5,
	6,7
};

unsigned int VAO, VBO;
//Framebuffer to do the shadow calculations
unsigned int lightBuff;
//Light buffer texture 3D
unsigned int texLightBuffer;
glm::mat4 mvp;
glm::mat4 lightmvp;
glm::mat4 modelMat = glm::mat4(1.0f);
//List to contain the final intersection points
std::vector<glm::vec3> intersectionList;
std::vector<glm::vec3> tempIntersectList;
//Vector halfway between the camera view direction and the light direction
glm::vec3 halfwayVec;


//FORWARD DECLARATIONS
GLubyte* load_3d_raw_data(std::string texture_path, glm::vec3 dimension);
glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t);
void loadModel();
void setupGUI(Screen* screen);
void sliceCubeView(glm::vec3 viewDirection);


int main()
{
	//Initialize glfw
	glfwInit();
	//Set required options
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(width, height, "Final Project", nullptr, nullptr);
	//Checking for window creation error
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Init GLEW to setup OpenGL function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	//Checking for GLAD
#if defined(NANOGUI_GLAD)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Could not initialize GLAD!");
		glGetError();
#endif

	//Setting screen color
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Create a nanogui screen
	screen = new Screen();
	screen->initialize(window, true);

	//Setting window details
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSwapInterval(0);
	glfwSwapBuffers(window);

	//GUI CODE
	setupGUI(screen);
	screen->setVisible(true);

	//Set up callbacks
	glfwSetCursorPosCallback(window,
		[](GLFWwindow*, double x, double y) {
			screen->cursorPosCallbackEvent(x, y);
		}
	);

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow*, int button, int action, int modifiers) {
			screen->mouseButtonCallbackEvent(button, action, modifiers);
		}
	);

	glfwSetKeyCallback(window,
		[](GLFWwindow*, int key, int scancode, int action, int mods) {
			screen->keyCallbackEvent(key, scancode, action, mods);
		}
	);

	glfwSetCharCallback(window,
		[](GLFWwindow*, unsigned int codepoint) {
			screen->charCallbackEvent(codepoint);
		}
	);

	glfwSetDropCallback(window,
		[](GLFWwindow*, int count, const char** filenames) {
			screen->dropCallbackEvent(count, filenames);
		}
	);

	glfwSetScrollCallback(window,
		[](GLFWwindow*, double x, double y) {
			screen->scrollCallbackEvent(x, y);
		}
	);

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow*, int width, int height) {
			screen->resizeCallbackEvent(width, height);
		}
	);

	//VBO and VAO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)sizeof(glm::vec3));
	glBindVertexArray(0);

	//Light buffer
	glGenFramebuffers(1, &lightBuff);
	glBindFramebuffer(GL_FRAMEBUFFER, lightBuff);

	//Light TExture
	glGenTextures(1, &texLightBuffer);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texLightBuffer);

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Setting the texture to clamp to border so if the object is no longer in the view of the light,
	//it will be in shadow
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//Attaching the texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texLightBuffer, 0);

	//Checking if framebuffer is complete
	/*if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Frame buffer is complete" << std::endl;
	else
		std::cout << "Frame buffer is not complete" << std::endl;*/

	//Creating the shader
	Shader shader("./Shaders/shader");
	//Creating the light_shader
	Shader lightShader("./Shaders/light_shader");
	//Getting uniform ids
	int mvpID = glGetUniformLocation(shader.m_program, "MVP");
	int viewID = glGetUniformLocation(shader.m_program, "viewMat");
	int projID = glGetUniformLocation(shader.m_program, "projMat");
	int userColorID = glGetUniformLocation(shader.m_program, "userColor");
	int tfsID = glGetUniformLocation(shader.m_program, "tfs");

	//Getting the ids for the transfer function variables
	int s0ID = glGetUniformLocation(shader.m_program, "s0");
	int s1ID = glGetUniformLocation(shader.m_program, "s1");
	int s2ID = glGetUniformLocation(shader.m_program, "s2");
	int s3ID = glGetUniformLocation(shader.m_program, "s3");
	int s4ID = glGetUniformLocation(shader.m_program, "s4");
	int s5ID = glGetUniformLocation(shader.m_program, "s5");
	int s6ID = glGetUniformLocation(shader.m_program, "s6");
	int s7ID = glGetUniformLocation(shader.m_program, "s7");
	//Same for the lightShader
	int ls0ID = glGetUniformLocation(lightShader.m_program, "s0");
	int ls1ID = glGetUniformLocation(lightShader.m_program, "s1");
	int ls2ID = glGetUniformLocation(lightShader.m_program, "s2");
	int ls3ID = glGetUniformLocation(lightShader.m_program, "s3");
	int ls4ID = glGetUniformLocation(lightShader.m_program, "s4");
	int ls5ID = glGetUniformLocation(lightShader.m_program, "s5");
	int ls6ID = glGetUniformLocation(lightShader.m_program, "s6");
	int ls7ID = glGetUniformLocation(lightShader.m_program, "s7");

	//Getting the id for the light mvp matrix
	int lightmvpID = glGetUniformLocation(shader.m_program, "lightmvp");
	int lightViewID = glGetUniformLocation(shader.m_program, "lightView");
	int lightModelID = glGetUniformLocation(shader.m_program, "lightModel");
	int lightProjectionID = glGetUniformLocation(shader.m_program, "lightProj");
	int lmvpID = glGetUniformLocation(lightShader.m_program, "MVP");
	int lViewID = glGetUniformLocation(lightShader.m_program, "view");
	int lModelID = glGetUniformLocation(lightShader.m_program, "model");
	int lProjID = glGetUniformLocation(lightShader.m_program, "projection");

	//Getting the ids for the light height and width
	int heightID = glGetUniformLocation(shader.m_program, "height");
	int widthID = glGetUniformLocation(shader.m_program, "width");

	//Generating texture ids
	glGenTextures(1, &tex3D);
	glGenTextures(1, &colorBarID);

	glDisable(GL_CULL_FACE);
	
	//RENDER LOOP
	while (!glfwWindowShouldClose(window))
	{
		//Clearing the lightBuff to be the light color
		glBindFramebuffer(GL_FRAMEBUFFER, lightBuff);
		glClearColor(light.color.r(), light.color.g(), light.color.b(), 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Back to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		//Calculating matrices
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, camera.zNear, camera.zFar);
		glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 40.0f);
		mvp = projection * camera.GetViewMatrix() * modelMat;
		lightmvp = lightProjection * glm::lookAt(light.position, light.position + light.front, light.up) * modelMat;

		//Clearing the default frame buffer
		//Alpha is set to 0 because the under operator blending won't show up if it isn't
		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO);		
		//BINDING SHADER
		shader.Bind();
		//Setting uniforms
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		glUniformMatrix4fv(projID, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3fv(userColorID, 1, objColor.data());
		glUniform1i(tfsID, transSign);

		//Setting transfer function variable uniforms
		glUniform1f(s0ID, s0);
		glUniform1f(s1ID, s1);
		glUniform1f(s2ID, s2);
		glUniform1f(s3ID, s3);
		glUniform1f(s4ID, s4);
		glUniform1f(s5ID, s5);
		glUniform1f(s6ID, s6);
		glUniform1f(s7ID, s7);

		//Setting lightmvp matrix
		glUniformMatrix4fv(lightmvpID, 1, GL_FALSE, glm::value_ptr(lightmvp));
		glUniformMatrix4fv(lightViewID, 1, GL_FALSE, glm::value_ptr(glm::lookAt(light.position, 
			light.position + light.front, light.up)));
		glUniformMatrix4fv(lightModelID, 1, GL_FALSE, glm::value_ptr(modelMat));
		glUniformMatrix4fv(lightProjectionID, 1, GL_FALSE, glm::value_ptr(lightProjection));
		//Setting width and height for the light
		glUniform1i(heightID, height);
		glUniform1i(widthID, width);
		

		//Activating and binding the textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, tex3D);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, colorBarID);

		//Running the algorithm to slice the cube and send the data to opengl
		sliceCubeView(camera.Front);

		//RENDER CODE
		if (transSign) {
			//Enable alpha blending
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			for (int i = 0; i < intersectionList.size(); i += 3)
			{
				if (backToFront)
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				else
					glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
				shader.Bind();
				glDrawArrays(GL_TRIANGLES, i, 3);
				//Binding the light buffer
				glBindFramebuffer(GL_FRAMEBUFFER, lightBuff);
				//Rendering the shadow slices
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				lightShader.Bind();
				//Setting lightmvp matrix
				glUniformMatrix4fv(lmvpID, 1, GL_FALSE, glm::value_ptr(mvp));
				glUniformMatrix4fv(lViewID, 1, GL_FALSE, glm::value_ptr(glm::lookAt(light.position, light.position + light.front, light.up)));
				glUniformMatrix4fv(lModelID, 1, GL_FALSE, glm::value_ptr(modelMat));
				glUniformMatrix4fv(lProjID, 1, GL_FALSE, glm::value_ptr(lightProjection));
				glUniform1f(ls0ID, s0);
				glUniform1f(ls1ID, s1);
				glUniform1f(ls2ID, s2);
				glUniform1f(ls3ID, s3);
				glUniform1f(ls4ID, s4);
				glUniform1f(ls5ID, s5);
				glUniform1f(ls6ID, s6);
				glUniform1f(ls7ID, s7);
				glDrawArrays(GL_TRIANGLES, i, 3);
				//unbinding the light buffer
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			glDisable(GL_BLEND);
		}
		else {
			if (render == LINE)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawArrays(GL_TRIANGLES, 0, intersectionList.size());
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else if (render == TRIANGLE)
			{
				glDrawArrays(GL_TRIANGLES, 0, intersectionList.size());
			}
			else if (render == POINT)
			{
				glDrawArrays(GL_POINTS, 0, intersectionList.size());
			}
		}
		glBindVertexArray(0);

		glfwPollEvents();

		//Draw gui
		screen->drawWidgets();
		//Swap screen buffersd
		glfwSwapBuffers(window);
	}
	//CLEAN UP RESOURCES
	//Delete buffers
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteFramebuffers(1, &lightBuff);
	glDeleteTextures(1, &texLightBuffer);
	glDeleteTextures(1, &tex3D);
	glDeleteTextures(1, &colorBarID);
	//deallocate glfw
	glfwTerminate();
	return 0;
}

GLubyte* load_3d_raw_data(std::string texture_path, glm::vec3 dimension) {
	size_t size = dimension[0] * dimension[1] * dimension[2];

	FILE* fp;
	GLubyte* data = new GLubyte[size];			  // 8bit
	if (!(fp = fopen(texture_path.c_str(), "rb"))) {
		std::cout << "Error: opening .raw file failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "OK: open .raw file successed" << std::endl;
	}
	if (fread(data, sizeof(char), size, fp) != size) {
		std::cout << "Error: read .raw file failed" << std::endl;
		exit(1);
	}
	else {
		std::cout << "OK: read .raw file successed" << std::endl;
	}
	fclose(fp);
	return data;
}

glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t)
{
	return x * (1.0f - t) + y * t;
}

void loadModel()
{
	//Generating, loading, and binding texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex3D);
	//Checking which model needs to be loaded
	if (model == HEAD)
	{
		GLubyte* data = load_3d_raw_data("./data_and_code/Head_256_256_225.raw", glm::vec3(256, 256, 225));
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, 256, 256, 225, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	else if (model == TEAPOT) {
		GLubyte* data = load_3d_raw_data("./data_and_code/BostonTeapot_256_256_178.raw", glm::vec3(256, 256, 178));
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, 256, 256, 178, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		
	}
	else if (model == BUCKY)
	{
		GLubyte* data = load_3d_raw_data("./data_and_code/Bucky_32_32_32.raw", glm::vec3(32, 32, 32));
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, 32, 32, 32, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	else if (model == BONSAI)
	{
		GLubyte* data = load_3d_raw_data("./data_and_code/Bonsai_512_512_154.raw", glm::vec3(512, 512, 154));
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, 512, 512, 154, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	// set the texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//set the mipmap levels (base and max)
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);
	//generate mipmaps
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);

	//glUniform1i(glGetUniformLocation(shader.m_program, "colorBar"), 1);
	//Loading the color bar
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, colorBarID);

	int barWidth, barHeight, nrComponents;
	//stbi_set_flip_vertically_on_load(true);
	unsigned char* barData = stbi_load(colorPath.c_str(), &barWidth, &barHeight, &nrComponents, 3);
	if (barData)
	{
		// set the texture parameters
		//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, barWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, barData);
		stbi_info("data_and_code/colorbar.png", &barWidth, &barHeight, &nrComponents);
		std::cout << "width: " << barWidth << " height: " << barHeight << " components: " << nrComponents << std::endl;
		stbi_image_free(barData);
	}
	else
	{
		std::cout << "Texture failed to load" << std::endl;
		stbi_image_free(barData);
	}
	glBindTexture(GL_TEXTURE_1D, 0);
}

void sliceCubeView(glm::vec3 viewDirection)
{
	//DEBUG
	//std::cout << "slicing" << std::endl;
	//Setting up to find the seperation
	float maxDist;
	float minDist;
	int maxIndex = 0;
	int minIndex = 0;
	int count = 0;

	//Finding the halfway vector between the view and light
	
	if (glm::dot(glm::normalize(viewDirection), glm::normalize(light.front)) >= 0)
	{
		//UNDEROPERATOR
		backToFront = false;
		halfwayVec = glm::normalize(glm::normalize(viewDirection) + glm::normalize(light.front));
	}
	else
	{
		//OVEROPERATOR
		backToFront = true;
		halfwayVec = glm::normalize(glm::normalize(-viewDirection) + glm::normalize(light.front));
	}

	//Setting initial maxDist
	maxDist = glm::dot(glm::normalize(halfwayVec), glm::normalize(test_cube_verts[0]));
	minDist = maxDist;
	//Finding the minimum and maximum distance of the vertices
	for (int i = 1; i < 8; i++)
	{
		//Find the distance indirectly byfinding the angle between the two vectors
		float dist = glm::dot(glm::normalize(halfwayVec), glm::normalize(test_cube_verts[i]));

		if (dist > maxDist)
		{
			maxDist = dist;
			maxIndex = i;
		}
		//Finding if there is an equivalent distance vertex on an edge from the minIndex vertex
		else if (dist == maxDist && glm::length(cube_verts[i] - cube_verts[minIndex]) < glm::length(cube_verts[maxIndex] - cube_verts[minIndex]))
		{
			maxIndex = i;
		}
		if (dist < minDist)
		{
			minDist = dist;
			minIndex = i;
		}
		//Finding if there is an equivalent distance vertex on an edge from the maxIndex vertex
		else if (dist == minDist && glm::length(cube_verts[i] - cube_verts[maxIndex]) < glm::length(cube_verts[minIndex] - cube_verts[maxIndex]))
		{
			minIndex = i;
		}
	}
	//Finding the seperation between slices
	float epsilon = .01;
	float seperation = (glm::length((cube_verts[maxIndex] - cube_verts[minIndex])) - epsilon)/ (sampleRate+1);

	//for each slice, using glm::rayintersectplane to find which edges are intersected
	intersectionList.clear();
	for (int i = 0; i < sampleRate; i++)
	{
		tempIntersectList.clear();
		//Finding the point on the slicing plane
		glm::vec3 planePoint;
		if (glm::dot(glm::normalize(viewDirection), glm::normalize(light.front)) < 0)
		{
			planePoint = cube_verts[minIndex] + ((seperation * i) * (glm::normalize(halfwayVec)));
		}
		else
		{
			planePoint = cube_verts[minIndex] + ((seperation * i) * (glm::normalize(halfwayVec)));
		}
		
		//Using the point on the plane to iterate over the edges and find which ones are intersected
		for (int k = 0; k < 12; k++)
		{
			glm::vec3 firstVert = cube_verts[cube_edges[k * 2]];
			glm::vec3 secondVert = cube_verts[cube_edges[k * 2 + 1]];
			//Finding intersected edges by using glm::rayintersectplane
			float intersectDistance = 0;
			glm::vec3 rayDir = (secondVert - firstVert);
			bool intersect = false;
			bool reverseIntersect = false;
			intersect = glm::intersectRayPlane(firstVert, rayDir, planePoint, halfwayVec, intersectDistance);
			glm::vec3 intersectPoint;
			//If there wasn't an intersection, check if there is in the other direction
			if (!intersect)
			{
				reverseIntersect = glm::intersectRayPlane(secondVert, -rayDir, planePoint, halfwayVec, intersectDistance);
			}
			//Limiting to intersections in the bounding cube
			if (intersect && intersectDistance <= 1 && intersectDistance >= 0)
			{
				intersectPoint = firstVert + intersectDistance * rayDir;
				tempIntersectList.push_back(intersectPoint);
			}
			//Same as above
			else if (reverseIntersect && intersectDistance <= 1 && intersectDistance >= 0)
			{
				intersectPoint = secondVert + intersectDistance * (-rayDir);
				tempIntersectList.push_back(intersectPoint);
			}
		}
		if (tempIntersectList.size() > 0) {
			//Finding the left most, bottom most vertex
			for (int k = 1; k < tempIntersectList.size(); k++)
			{
				if (tempIntersectList.at(k).x < tempIntersectList.at(0).x)
					std::swap(tempIntersectList[k], tempIntersectList[0]);
			}
			for (int k = 1; k < tempIntersectList.size(); k++)
			{
				if (tempIntersectList.at(k).x == tempIntersectList.at(0).x
					&& tempIntersectList.at(k).y < tempIntersectList.at(0).y)
					std::swap(tempIntersectList[k], tempIntersectList[0]);
			}
			//Finding a middle vertex by averaging intersection points
			glm::vec3 middlePoint = tempIntersectList.at(0);
			for (int k = 1; k < tempIntersectList.size(); k++)
			{
				middlePoint = middlePoint + tempIntersectList.at(k);
			}
			middlePoint = middlePoint / tempIntersectList.size();
			//Sorting vertices based on angle. The angle from tempIntersectList[x] to tempIntersectList[x+1] will be the smallest possible
			//Calculating angle by using the middlepoint to form angles
			for (int k = 0; k < tempIntersectList.size() - 1; k++)
			{
				glm::vec3 midToPoint = middlePoint - tempIntersectList.at(k);
				glm::vec3 midToNext = middlePoint - tempIntersectList.at(k + 1);
				float angleBetween = glm::angle(glm::normalize(midToPoint), glm::normalize(midToNext));
				for (int j = k + 2; j < tempIntersectList.size(); j++)
				{
					glm::vec3 midToTest = middlePoint - tempIntersectList.at(j);
					float testAngle = glm::angle(glm::normalize(midToPoint), glm::normalize(midToTest));
					if (testAngle < angleBetween)
					{
						std::swap(testAngle, angleBetween);
						std::swap(tempIntersectList[k + 1], tempIntersectList[j]);
					}
				}
			}

			for (int k = 0; k < tempIntersectList.size() - 2; k++)
			{
				intersectionList.push_back(tempIntersectList.at(0));
				intersectionList.push_back(tempIntersectList.at(k + 1));
				intersectionList.push_back(tempIntersectList.at(k + 2));
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* intersectionList.size(), &intersectionList[0].x, GL_DYNAMIC_DRAW);

	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void setupGUI(Screen* screen)
{
	FormHelper* gui = new nanogui::FormHelper(screen);
	//Control bar one, for moving the camera and setting configuration
	ref<nanogui::Window> controlOne = gui->addWindow(Eigen::Vector2i(0.0f, 0.0f), "Control Bar 1");
	//Group for controlling camera position
	gui->addGroup("Position");
	gui->addVariable("X", camera.Position.x)->setSpinnable(true);
	gui->addVariable("Y", camera.Position.y)->setSpinnable(true);
	gui->addVariable("Z", camera.Position.z)->setSpinnable(true);
	//Group for controlling camera rotation
	gui->addGroup("Rotate");
	gui->addVariable("Rotate value", rotateVal);
	gui->addButton("Rotate right+", []
		{ camera.rotateCameraX((float)rotateVal); });
	gui->addButton("Rotate right-", []
		{ camera.rotateCameraX((float)-rotateVal); });
	gui->addButton("Rotate up+", []
		{ camera.rotateCameraY((float)rotateVal); });
	gui->addButton("Rotate up-", []
		{ camera.rotateCameraY((float)-rotateVal); });
	gui->addButton("Rotate front+", []
		{ camera.rotateCameraZ((float)rotateVal); });
	gui->addButton("Rotate front-", []
		{ camera.rotateCameraZ((float)-rotateVal); });
	//Group for configuring the model
	gui->addGroup("Configuration");
	gui->addVariable("Model name", model, true)->setItems({ "HEAD", "TEAPOT", "BUCKY", "BONSAI" });
	gui->addVariable("Render type", render, true)->setItems({ "LINE", "TRIANGLE", "POINT" });
	gui->addVariable("Colorbar image path", colorPath);
	gui->addButton("Reload model", []
		{ loadModel(); });
	gui->addButton("Reset camera", [gui]
		{ camera.reset();
		gui->refresh(); });
	//Group to change volume rendering details
	gui->addGroup("Volume Rendering");
	gui->addVariable("Object color", objColor);
	gui->addVariable("Transfer function sign", transSign);
	gui->addVariable("Sampling rate s(unit slice number)", sampleRate);

	//Window for controlling the light
	FormHelper* gui_light = new FormHelper(screen);
	ref<Window> lightControl = gui_light->addWindow(Eigen::Vector2i(10.0f, 0.0f), "Light Control");
	gui_light->addVariable("X", light.position.x)->setSpinnable(true);
	gui_light->addVariable("Y", light.position.y)->setSpinnable(true);
	gui_light->addVariable("Z", light.position.z)->setSpinnable(true);
	gui_light->addGroup("Light Direction");
	gui_light->addVariable("Light Color", light.color);
	gui_light->addVariable("Light Front X", light.front.x);
	gui_light->addVariable("Light Front Y", light.front.y);
	gui_light->addVariable("Light Front Z", light.front.z);
	gui_light->addVariable("Rotate Val", lightRotateVal)->setSpinnable(true);
	gui_light->addButton("Rotate right+", [gui_light]
		{ light.rotateLightX((float)lightRotateVal);
			//rotating pointer model too
			gui_light->refresh(); });
	gui_light->addButton("Rotate right-", [gui_light]
		{ light.rotateLightX((float)-lightRotateVal);
			gui_light->refresh(); });
	gui_light->addButton("Rotate up+", [gui_light]
		{ light.rotateLightY((float)lightRotateVal);
			gui_light->refresh(); });
	gui_light->addButton("Rotate up-", [gui_light]
		{ light.rotateLightY((float)-lightRotateVal);
			gui_light->refresh(); });
	gui_light->addButton("Rotate front+", [gui_light]
		{ light.rotateLightZ((float)lightRotateVal);
			gui_light->refresh(); });
	gui_light->addButton("Rotate front-", [gui_light]
		{ light.rotateLightZ((float)-lightRotateVal);
			gui_light->refresh(); });
	gui_light->addButton("Reset Light", [gui_light]
		{
			light.reset();
			//Resetting pointerModel
			gui_light->refresh();
		});
	gui_light->addGroup("halfway vec");
	gui_light->addVariable("X", halfwayVec.x)->setEditable(false);
	gui_light->addVariable("Y", halfwayVec.y)->setEditable(false);
	gui_light->addVariable("Z", halfwayVec.z)->setEditable(false);

	//Control bar two for configuring the transfer function
	FormHelper* gui_2 = new FormHelper(screen);
	ref<nanogui::Window> controlTwo = gui_2->addWindow(Eigen::Vector2i(0.0f, 0.0f), "Control Bar 2");
	//screen->performLayout();
	Widget* panel1 = new Widget(controlTwo);
	gui_2->addWidget("View Slider", panel1);
	panel1->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 20));

	Slider* slider1 = new Slider(panel1);
	slider1->setValue(camera.zNear);
	slider1->setFixedWidth(80);
	slider1->setPosition(Eigen::Vector2i(250, 100));

	TextBox* textBox1 = new TextBox(panel1);
	textBox1->setFixedSize(Eigen::Vector2i(100, 25));
	textBox1->setValue("0.0");
	slider1->setCallback([textBox1](float value) {
		textBox1->setValue(std::to_string((float)(value)));
		camera.zNear = 0.5f;
		camera.zFar = 200.0f;
		});
	//graph
	nanogui::Widget* graphWidget = new nanogui::Widget(controlTwo);
	graphWidget->setHeight(100);

	//Creating the graph
	gui_2->addWidget("Transfer function", graphWidget);
	nanogui::Graph* graph = graphWidget->add<nanogui::Graph>("Alpha");
	graph->setFixedHeight(100);
	nanogui::VectorXf& func = graph->values();
	func.resize(256);
	graph->setFooter("Intensity");

	nanogui::Widget* panel2 = new nanogui::Widget(controlTwo);
	gui_2->addWidget("Slider 0", panel2);
	panel2->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	//Creating a bunch of sliders and textboxes to work with the graph
	nanogui::Slider* slider2 = new nanogui::Slider(panel2);
	slider2->setValue(s0);
	slider2->setFixedWidth(80);
	slider2->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox* textBox2 = new nanogui::TextBox(panel2);
	textBox2->setFixedSize(Eigen::Vector2i(100, 25));
	textBox2->setValue("0.0");
	slider2->setCallback([textBox2, &func](float value) {
		textBox2->setValue(std::to_string((float)(value * 1)));
		s0 = value;
		func[0] = s0;
		func[32] = s1;

		for (int i = 1; i < 32; i++)
		{
			func[i] = s0 + (s1 - s0) * i / 32;
		}

		});


	nanogui::Widget* panel3 = new nanogui::Widget(controlTwo);
	gui_2->addWidget("Slider 1", panel3);
	panel3->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider* slider3 = new nanogui::Slider(panel3);
	slider3->setValue(s1);
	slider3->setFixedWidth(80);
	slider3->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox* textBox3 = new nanogui::TextBox(panel3);
	textBox3->setFixedSize(Eigen::Vector2i(100, 25));
	textBox3->setValue("0.0");
	slider3->setCallback([textBox3, &func](float value) {
		textBox3->setValue(std::to_string((float)(value * 1)));
		s1 = value;
		func[0] = s0;
		func[32] = s1;
		func[64] = s2;

		//s0-s1
		for (int i = 1; i < 32; i++)
		{
			func[i] = s0 + (s1 - s0) * i / 32;
		}

		//s1-s2
		for (int i = 33; i < 63; i++)
		{
			func[i] = s1 + (s2 - s1) * (i - 32) / 32;
		}

		});
	nanogui::Widget* panel4 = new nanogui::Widget(controlTwo);
	gui_2->addWidget("Slider 2", panel4);
	panel4->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider* slider4 = new nanogui::Slider(panel4);
	slider4->setValue(s2);
	slider4->setFixedWidth(80);
	slider4->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox* textBox4 = new nanogui::TextBox(panel4);
	textBox4->setFixedSize(Eigen::Vector2i(100, 25));
	textBox4->setValue("0.0");
	slider4->setCallback([textBox4, &func](float value) {
		textBox4->setValue(std::to_string((float)(value * 1)));
		s2 = value;
		func[32] = s1;
		func[64] = s2;
		func[96] = s3;

		//s1-s2
		for (int i = 33; i < 64; i++)
		{
			func[i] = s1 + (s2 - s1) * (i - 32) / 32;
		}

		//s2-s3
		for (int i = 65; i < 95; i++)
		{
			func[i] = s2 + (s3 - s2) * (i - 64) / 32;
		}

		});
	nanogui::Widget* panel5 = new nanogui::Widget(controlTwo);
	gui_2->addWidget("Slider 3", panel5);
	panel5->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider* slider5 = new nanogui::Slider(panel5);
	slider5->setValue(s3);
	slider5->setFixedWidth(80);
	slider5->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox* textBox5 = new nanogui::TextBox(panel5);
	textBox5->setFixedSize(Eigen::Vector2i(100, 25));
	textBox5->setValue("0.0");
	slider5->setCallback([textBox5, &func](float value) {
		textBox5->setValue(std::to_string((float)(value * 1)));
		s3 = value;
		func[64] = s2;
		func[96] = s3;
		func[128] = s4;


		//s2-s3
		for (int i = 65; i < 96; i++)
		{
			func[i] = s2 + (s3 - s2) * (i - 64) / 32;
		}

		//s3-s4
		for (int i = 97; i < 128; i++)
		{
			func[i] = s3 + (s4 - s3) * (i - 96) / 32;
		}

		});

	nanogui::Widget* panel6 = new nanogui::Widget(controlTwo);
	gui_2->addWidget("Slider 4", panel6);
	panel6->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider* slider6 = new nanogui::Slider(panel6);
	slider6->setValue(s4);
	slider6->setFixedWidth(80);
	slider6->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox* textBox6 = new nanogui::TextBox(panel6);
	textBox6->setFixedSize(Eigen::Vector2i(100, 25));
	textBox6->setValue("0.0");
	slider6->setCallback([textBox6, &func](float value) {
		textBox6->setValue(std::to_string((float)(value * 1)));
		s4 = value;
		func[96] = s3;
		func[128] = s4;
		func[160] = s5;

		//s3-s4
		for (int i = 97; i < 128; i++)
		{
			func[i] = s3 + (s4 - s3) * (i - 96) / 32;
		}

		//s4-s5
		for (int i = 129; i < 160; i++)
		{
			func[i] = s4 + (s5 - s4) * (i - 128) / 32;
		}
		});

	nanogui::Widget* panel7 = new nanogui::Widget(controlTwo);
	gui_2->addWidget("Slider 5", panel7);
	panel7->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider* slider7 = new nanogui::Slider(panel7);
	slider7->setValue(s5);
	slider7->setFixedWidth(80);
	slider7->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox* textBox7 = new nanogui::TextBox(panel7);
	textBox7->setFixedSize(Eigen::Vector2i(100, 25));
	textBox7->setValue("0.0");
	slider7->setCallback([textBox7, &func](float value) {
		textBox7->setValue(std::to_string((float)(value * 1)));
		s5 = value;
		func[128] = s4;
		func[160] = s5;
		func[192] = s6;

		//s4-s5
		for (int i = 129; i < 160; i++)
		{
			func[i] = s4 + (s5 - s4) * (i - 128) / 32;
		}

		//s5-s6
		for (int i = 161; i < 192; i++)
		{
			func[i] = s5 + (s6 - s5) * (i - 160) / 32;
		}

		});
	nanogui::Widget* panel8 = new nanogui::Widget(controlTwo);
	gui_2->addWidget("Slider 6", panel8);
	panel8->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider* slider8 = new nanogui::Slider(panel8);
	slider8->setValue(s6);
	slider8->setFixedWidth(80);
	slider8->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox* textBox8 = new nanogui::TextBox(panel8);
	textBox8->setFixedSize(Eigen::Vector2i(100, 25));
	textBox8->setValue("0.0");
	slider8->setCallback([textBox8, &func](float value) {
		textBox8->setValue(std::to_string((float)(value * 1)));
		s6 = value;
		func[160] = s5;
		func[192] = s6;
		func[225] = s7;

		//s5-s6
		for (int i = 161; i < 192; i++)
		{
			func[i] = s5 + (s6 - s5) * (i - 160) / 32;
		}

		//s6-s7
		for (int i = 193; i < 225; i++)
		{
			func[i] = s6 + (s7 - s6) * (i - 192) / 32;
		}


		});

	//slider 9
	nanogui::Widget* panel9 = new nanogui::Widget(controlTwo);
	gui_2->addWidget("Slider 7", panel9);
	panel9->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider* slider9 = new nanogui::Slider(panel9);
	slider9->setValue(s7);
	slider9->setFixedWidth(80);
	slider9->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox* textBox9 = new nanogui::TextBox(panel9);
	textBox9->setFixedSize(Eigen::Vector2i(100, 25));
	textBox9->setValue("0.0");
	slider9->setCallback([textBox9, &func](float value) {
		textBox9->setValue(std::to_string((float)(value * 1)));
		s7 = value;
		func[192] = s6;
		func[225] = s7;
		func[255] = s8;

		//s6-s7
		for (int i = 193; i < 225; i++)
		{
			func[i] = s6 + (s7 - s6) * (i - 192) / 32;
		}

		//s7-s8
		for (int i = 226; i < 256; i++)
		{
			func[i] = s7 + (s8 - s7) * (i - 225) / 32;
		}


		});
	screen->performLayout();
}