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

#include "Game.h"


int main(void)
{
	try {
		Game game(2560, 1440, "Voxel Engine");
		game.Run();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}