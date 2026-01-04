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



void Skybox::Draw(const glm::mat4& view, const glm::mat4& projection)
{
    GLCall(glDepthFunc(GL_LEQUAL));

    m_Shader.Bind();

    m_Shader.SetUniformMat4f("u_View", glm::mat4(glm::mat3(view)));
    m_Shader.SetUniformMat4f("u_Proj", projection);

    GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID));
    m_Shader.SetUniform1i("skybox", 0);

    m_VA.Bind();
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 36));

    GLCall(glDepthFunc(GL_LESS));
}