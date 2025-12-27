#include "Input.h"

int Input::GetHorizontalMoveInput()
{
	int left = glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS ? -1 : 0;
	int right = glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS ? 1 : 0;
	return left + right;
}

int Input::GetVerticalMoveInput()
{
	int forward = glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS ? 1 : 0;
	int backward = glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS ? -1 : 0;
	return forward + backward;
}

bool Input::IsKeyPressed(int key)
{
	return glfwGetKey(m_Window, key) == GLFW_PRESS;
}

void Input::UpdateMouse()
{
    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);

    if (m_FirstMouse)
    {
        m_LastMouseX = xpos;
        m_LastMouseY = ypos;
        m_FirstMouse = false;
    }

    m_MouseDelta.x = float(xpos - m_LastMouseX);
    m_MouseDelta.y = float(m_LastMouseY - ypos);

    m_LastMouseX = xpos;
    m_LastMouseY = ypos;
}

Vector2f Input::GetMouseInput()
{
    return m_MouseDelta;
}