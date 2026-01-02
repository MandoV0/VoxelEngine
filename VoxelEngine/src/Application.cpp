#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Shader.h"
#include "texture.h"

#include <ext/matrix_clip_space.hpp>
#include <ext/matrix_transform.hpp>
#include "Input.h"
#include "Camera.h"
#include "world/World.h"


int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Error!" << std::endl;
	}

	{
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);

		Shader shader("res/shaders/vertex.shader", "res/shaders/fragment.shader");
		shader.Bind();

		Renderer renderer;

		std::string texturePath = "res/textures/atlas.png";
		Texture texture(texturePath);
		texture.Bind();
		shader.SetUniform1i("u_Texture", 0);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 100.0f);

		Input input(window);
		Camera camera(glm::vec3(8.0f, 5.0f, 8.0f));
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		float lastFrame = 0.0f;

		World world(1337);

		for (int x = -1; x <= 1; x++)
		{
			for (int z = -1; z <= 1; z++)
			{
				world.GenerateChunk(x, z);
			}
		}

		// Crosshair Setup
		float crosshairVertices[] = {
			-0.03f,  0.0f, 0.0f,  0.0f, 0.0f,
			 0.03f,  0.0f, 0.0f,  1.0f, 1.0f,
			 0.0f, -0.04f, 0.0f,  0.0f, 0.0f,
			 0.0f,  0.04f, 0.0f,  1.0f, 1.0f
		};
		unsigned int crosshairIndices[] = { 0, 1, 2, 3 };

		VertexArray chVA;
		VertexBuffer chVB(crosshairVertices, sizeof(crosshairVertices));
		VertexBufferLayout chLayout;
		chLayout.Push<float>(3);
		chLayout.Push<float>(2);
		chVA.AddBuffer(chVB, chLayout);
		IndexBuffer chIB(crosshairIndices, 4);

		float clickTimer = 0.0f;
		float clickCooldown = 0.15f;

		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = glfwGetTime();
			float deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			clickTimer -= deltaTime;

			renderer.Clear();

			input.UpdateMouse();

			bool leftMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
			bool rightMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
			bool canClick = clickTimer <= 0.0f;

			camera.ProcessKeyboard(input, deltaTime);
			camera.ProcessMouse(input.GetMouseInput());

			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 mvp = proj * view * model;

			shader.Bind();
			shader.SetUniformMat4f("u_MVP", glm::value_ptr(mvp));
			texture.Bind();
			shader.SetUniform1i("u_Texture", 0);

			world.Render(renderer, shader);

			// Render Crosshair
			glDisable(GL_DEPTH_TEST);
			shader.Bind();
			glm::mat4 identity = glm::mat4(1.0f);
			shader.SetUniformMat4f("u_MVP", glm::value_ptr(identity));

			chVA.Bind();
			chIB.Bind();
			glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, nullptr);
			glEnable(GL_DEPTH_TEST);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	glfwTerminate();
	return 0;
}