#pragma once

#include <glm.hpp>
#include "Input.h"
#include <ext/matrix_transform.hpp>
#include "CameraFrustum.h"

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

	float m_NearPlane = 0.1f;
	float m_FarPlane = 1000.0f;

	float m_Fov = 85.0f;
	float m_AspectRatio;

	CameraFrustum m_Frustum;

public:
	Camera(glm::vec3 position = glm::vec3(0, 0, 3), glm::vec3 up = glm::vec3(0, 1, 0), float yaw = -90.0f, float pitch = 0.0f, float aspectRatio = 0.0f, float fov = 85.0f);

	void ProcessKeyboard(Input& input, float deltaTime);
	void ProcessMouse(Vector2f mouseDelta);

	glm::mat4 GetViewMatrix() const { return glm::lookAt(m_Position, m_Position + m_Front, m_Up); }
	glm::vec3 GetPosition() const { return m_Position; }
	glm::vec3 GetFront() const { return m_Front; }
	glm::vec3 GetRight() const { return m_Right; }
	glm::vec3 GetUp() const { return m_Up; }

	bool FrustumIntersectsAABB(glm::vec3 boxMin, glm::vec3 boxMax) const;

private:
	void UpdateCameraVectors();
};