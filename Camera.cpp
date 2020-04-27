#include "Camera.h"

//CONSTRUCTOR
//default
// Constructor with vectors
Camera::Camera(glm::vec3 position) : Front(0.0f, 0.0f, -1.0f), Right(1.0f, 0.0f, 0.0f), Up(0.0f, 1.0f, 0.0f), zNear(0.5f), zFar(30.0f)
{
	Position = position;
}
//DESTRUCTOR
Camera::~Camera()
{

}

//GETTERS AND SETTERS
// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::GetViewMatrix()
{
	//updateCameraVectors();
	return glm::lookAt(Position, Position + Front, Up);
}

void Camera::rotateCameraX(float angle)
{
	glm::mat4 rotation_mat(1.0f);
	rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), Right);
	Up = glm::normalize(glm::vec3(rotation_mat * glm::vec4(Up, 1.0f)));
	Front = glm::normalize(glm::cross(Up, Right));
}
void Camera::rotateCameraY(float angle)
{
	glm::mat4 rotation_mat(1.0f);
	rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), Up);
	Front = glm::normalize(glm::vec3(rotation_mat * glm::vec4(Front, 1.0f)));
	Right = glm::normalize(glm::cross(Front, Up));
}
void Camera::rotateCameraZ(float angle)
{
	glm::mat4 rotation_mat(1.0f);
	rotation_mat = glm::rotate(rotation_mat, glm::radians(angle), Front);
	Right = glm::normalize(glm::vec3(rotation_mat * glm::vec4(Right, 1.0f)));
	Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::reset()
{
	Position = glm::vec3(0.5f, 0.5f, 3.0f);
	Front = glm::vec3(0.0f, 0.0f, -1.0f);
	Up = glm::vec3(0.0f, 1.0f, 0.0f);
	Right = glm::vec3(1.0f, 0.0f, 0.0f);
}
