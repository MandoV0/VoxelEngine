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
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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

		float vertices[] = {
			// front
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

			// back
			 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

			 // left
			 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			 -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

			 // right
			 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

			 // top
			 -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

			 // bottom
			 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			  0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			  0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
			 -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
		};


		unsigned int indices[] = {
			0,  1,  2,   2,  3,  0,		// Front
			4,  5,  6,   6,  7,  4,		// Back
			8,  9, 10,  10, 11,  8,		// Left
			12, 15, 14,  14, 13, 12,	// Right
			16, 17, 18,  18, 19, 16,	// Top
			20, 21, 22,  22, 23, 20		// Bottom
		};

		unsigned int vao;
		GLCall(glGenVertexArrays(1, &vao));
		GLCall(glBindVertexArray(vao));

		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);

		VertexArray va;
		VertexBuffer vb(vertices, sizeof(vertices)); // 24 Vertices, 5 floats per vertex, xyz + uv

		VertexBufferLayout layout;
		layout.Push<float>(3); // 3 floats for position
		layout.Push<float>(2); // 2 floats for texture coordinates / UV
		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices, 36);

		Shader shader("res/shaders/vertex.shader", "res/shaders/fragment.shader");
		shader.Bind();

		va.Bind();
		va.Unbind();
		ib.Unbind();

		Renderer renderer;

		std::string texturePath = "res/textures/cobblestone.png";

		Texture texture(texturePath);
		texture.Bind();
		shader.SetUniform1i("u_Texture", 0);
		shader.Unbind();

		shader.Bind();

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 100.0f);

		Input input(window);
		Camera camera(glm::vec3(2.0f, 2.0f, 2.0f));
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		float lastFrame = 0.0f;

		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = glfwGetTime();
			float deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			renderer.Clear();

			input.UpdateMouse();

			camera.ProcessKeyboard(input, deltaTime);
			camera.ProcessMouse(input.GetMouseInput());

			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 mvp = proj * view * model;

			shader.Bind();
			shader.SetUniformMat4f("u_MVP", glm::value_ptr(mvp));
			renderer.Draw(va, ib, shader);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	glfwTerminate();
	return 0;
}