#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float aspectRatio, float fov)
    : m_Position(position), m_WorldUp(up), m_Yaw(yaw), m_Pitch(pitch),
	m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_Speed(2.5f), m_Sensitivity(0.1f), m_Fov(fov), m_AspectRatio(aspectRatio)
{
    UpdateCameraVectors();
    m_Frustum.SetCamInternals(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
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

/*
* Imagine the camera frustum as a pyramid and we want to check if anything is inside that pyramid so we know if we should issue a draw call for it or not.
* 
* The camera frustum has 6 planes: near, far, left, right, top, bottom.
* Each plane can be represented by a normal (Pointing direction) and a distance from the origin/camera.
* We then check if any of the corners cross into our frustum by checking if they are in front of any of the 6 planes using the dot product.
*/
bool Camera::FrustumIntersectsAABB(glm::vec3 boxMin, glm::vec3 boxMax) const
{
    AABox box = { boxMin, boxMax };
	int result = m_Frustum.BoxInFrustum(box);

	return result != CameraFrustum::OUTSIDE;

    return false;
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

	m_Frustum.SetCamDef(m_Position, m_Position + m_Front, m_Up);
}
