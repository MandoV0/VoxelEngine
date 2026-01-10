#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_Position(position), m_WorldUp(up), m_Yaw(yaw), m_Pitch(pitch),
    m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_Speed(2.5f), m_Sensitivity(0.1f)
{
    UpdateCameraVectors();
}

void Camera::ProcessKeyboard(Input& input, float deltaTime)
{
    float velocity = m_Speed * deltaTime;
    
	if (input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
        velocity *= 35.0f; // Sprinting

    if (input.GetVerticalMoveInput() != 0)
        m_Position += m_Front * (float)input.GetVerticalMoveInput() * velocity;
    if (input.GetHorizontalMoveInput() != 0)
        m_Position += glm::normalize(glm::cross(m_Front, m_Up)) * (float)input.GetHorizontalMoveInput() * velocity;
}

void Camera::ProcessMouse(Vector2f mouseDelta)
{
    m_Yaw += mouseDelta.x * m_Sensitivity;
    m_Pitch += mouseDelta.y * m_Sensitivity;

    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
