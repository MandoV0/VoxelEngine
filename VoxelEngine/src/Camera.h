#pragma once

#include <glm.hpp>
#include "Input.h"
#include <ext/matrix_transform.hpp>

class Camera
{
private:
	/* TODO: Can be mostly generalized into a Transform component, this way we can then later use it for meshes */
	glm::vec3 m_Position;
	glm::vec3 m_Front;
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	glm::vec3 m_WorldUp;

	float m_Yaw;
	float m_Pitch;

	float m_Speed;
	float m_Sensitivity;
public:
	Camera(glm::vec3 position = glm::vec3(0, 0, 3), glm::vec3 up = glm::vec3(0, 1, 0), float yaw = -90.0f, float pitch = 0.0f);

	void ProcessKeyboard(Input& input, float deltaTime);
	void ProcessMouse(Vector2f mouseDelta);

	glm::mat4 GetViewMatrix() const { return glm::lookAt(m_Position, m_Position + m_Front, m_Up); }
	glm::vec3 GetPosition() const { return m_Position; }

private:
	void UpdateCameraVectors();
};