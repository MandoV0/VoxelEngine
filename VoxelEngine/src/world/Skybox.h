#pragma once
#include "../IndexBuffer.h"
#include "../VertexBuffer.h"
#include "../VertexArray.h"

#include "../Renderer.h"
#include <glm.hpp>

/*
* A big Cube that follows the camera to simulate a skybox.
* The faces are all facing inwards.
*/
class Skybox
{
private:
	unsigned int m_CubemapID;
	VertexArray m_VA;
	VertexBuffer* m_VB;
	Shader m_Shader;

public:
	Skybox(unsigned int cubemapID);
	~Skybox();

	void Draw(const glm::mat4& view, const glm::mat4&projection);
};