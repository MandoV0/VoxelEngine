#include "Skybox.h"

#include "../VertexBufferLayout.h"


Skybox::Skybox(unsigned int cubemapID) : m_Shader("res/shaders/sky_vertex.shader", "res/shaders/sky_fragment.shader")
{
	m_CubemapID = cubemapID;
    float skyboxVertices[] = {
        -1,  1, -1,  -1, -1, -1,   1, -1, -1,
         1, -1, -1,   1,  1, -1,  -1,  1, -1,

        -1, -1,  1,  -1, -1, -1,  -1,  1, -1,
        -1,  1, -1,  -1,  1,  1,  -1, -1,  1,

         1, -1, -1,   1, -1,  1,   1,  1,  1,
         1,  1,  1,   1,  1, -1,   1, -1, -1,

        -1, -1,  1,  -1,  1,  1,   1,  1,  1,
         1,  1,  1,   1, -1,  1,  -1, -1,  1,

        -1,  1, -1,   1,  1, -1,   1,  1,  1,
         1,  1,  1,  -1,  1,  1,  -1,  1, -1,

        -1, -1, -1,  -1, -1,  1,   1, -1, -1,
         1, -1, -1,  -1, -1,  1,   1, -1,  1
    };


	m_VB = new VertexBuffer(skyboxVertices, sizeof(skyboxVertices));
	VertexBufferLayout layout;
	layout.Push<float>(3);

	m_VA.AddBuffer(*m_VB, layout);
}

Skybox::~Skybox()
{
	delete m_VB;
}

void Skybox::Bind() const
{
	m_Shader.Bind();
}