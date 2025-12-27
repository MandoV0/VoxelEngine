#pragma once

#include <GLFW/glfw3.h>

struct Vector2f
{
	float x;
	float y;
};

class Input
{
private:
	GLFWwindow* m_Window;

	double m_LastMouseX = 0.0;
	double m_LastMouseY = 0.0;
	bool m_FirstMouse = true;
	Vector2f m_MouseDelta = { 0.0f, 0.0f };

public:
	Input(GLFWwindow* window) : m_Window(window) {}

	int GetHorizontalMoveInput();
	int GetVerticalMoveInput();
	bool IsKeyPressed(int key);

	void UpdateMouse();

	Vector2f GetMouseInput();
};