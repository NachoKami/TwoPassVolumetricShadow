/*	Programmer: NekoKami2345
	Date: 2/13/2020
	This is the header file for the Camera class,
	which contains a matrix for transforming world coords
	to view coords and view coords to clip coords,
	and methods that might be needed for a camera*/
#pragma once

//include glm for vec3
#include <glm/glm.hpp>
#include <glm/ext.hpp>
//Include quaternion stuff
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera
{
public:
	//CONSTRUCTOR
	//default
	Camera();
	Camera(glm::vec3 position);// : Front(0.0f, 0.0f, -1.0f), Right(1.0f, 0.0f, 0.0f), Up(0.0f, 1.0f, 0.0f), zNear(0.5f), zFar(30.0f);

	

	//DESTRUCTOR
	virtual ~Camera();

	//Matrices for view and projection
	glm::mat4 view;
	glm::mat4 projection;
	//Matrix of projection * view
	glm::mat4 cameraMat;

	//size of screen
	float width, height;
	//fov in degrees
	float fov;
	//znear and zfar values
	float zNear, zFar;

	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	float Roll;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix();
	//Functions to rotate the camera
	void rotateCameraX(float angle);
	void rotateCameraY(float angle);
	void rotateCameraZ(float angle);
	//Function to reset the camera
	void reset();
protected:
private:


	
};

