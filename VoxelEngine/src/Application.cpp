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
#include "world/Skybox.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


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

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");


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

		const float fov = 85.0f;
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 proj = glm::perspective(glm::radians(fov), 1920.0f / 1080.0f, 0.1f, 1200.0f);

		Input input(window);
		Camera camera(glm::vec3(8.0f, 5.0f, 8.0f));
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		float lastFrame = 0.0f;

		World world(1337);


		const int renderDistance = 1;
		for (int x = -renderDistance; x <= renderDistance; x++)
		{
			for (int z = -renderDistance; z <= renderDistance; z++)
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

		unsigned int cubeMapID = Texture::LoadCubemap("res/textures/Cubemap_Sky_04-512x512.png");;
		Skybox sb(cubeMapID);

		double lastTime = glfwGetTime();
		int nbFrames = 0;

		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = glfwGetTime();
			float deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			clickTimer -= deltaTime;

			double currentTime = glfwGetTime();
			nbFrames++;
			if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
				// printf and reset timer
				printf("%f ms/frame\n", 1000.0 / double(nbFrames));
				nbFrames = 0;
				lastTime += 1.0;
			}

			renderer.Clear();

			input.UpdateMouse();

			bool leftMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
			bool rightMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
			bool canClick = clickTimer <= 0.0f;

			
			glm::ivec3 hitBlock(0), placeBlock(0);


			camera.ProcessKeyboard(input, deltaTime);
			camera.ProcessMouse(input.GetMouseInput());

			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 mvp = proj * view * model;

			shader.Bind();
			shader.SetUniformMat4f("u_MVP", mvp);
			texture.Bind();
			shader.SetUniform1i("u_Texture", 0);

			world.Render(renderer, shader);

			if (world.Raycast(camera.GetPosition(), camera.GetFront(), 15.0f, hitBlock, placeBlock)){
				world.RenderBlockOutline(renderer, shader, hitBlock.x, hitBlock.y, hitBlock.z);

				if (canClick)
				{
					if (leftMouseDown)
					{
						world.SetBlock(hitBlock.x, hitBlock.y, hitBlock.z, BlockType::AIR);
						clickTimer = clickCooldown;
					}
					else if (rightMouseDown)
					{
						world.SetBlock(placeBlock.x, placeBlock.y, placeBlock.z, BlockType::GRASS);
						clickTimer = clickCooldown;
					}
				}
			}

			// Render Skybox
			sb.Draw(view, proj);

			// Render Crosshair
			glDisable(GL_DEPTH_TEST);
			shader.Bind();
			glm::mat4 identity = glm::mat4(1.0f);
			shader.SetUniformMat4f("u_MVP", identity);

			chVA.Bind();
			chIB.Bind();
			glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, nullptr);
			glEnable(GL_DEPTH_TEST);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			float cameraX = camera.GetPosition().x;
			float cameraY = camera.GetPosition().y;
			float cameraZ = camera.GetPosition().z;

			ImGui::Begin("ImGui");
			ImGui::SetWindowSize(ImVec2(300, 100), ImGuiCond_Always);
			ImGui::Text("Camera Position: X %.2f | Y %.2f | Z %.2f \nBlock Position: X %d | Y %d | Z %d", cameraX, cameraY, cameraZ, hitBlock.x, hitBlock.y, hitBlock.z);
			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}