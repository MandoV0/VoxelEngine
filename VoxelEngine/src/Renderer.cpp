#include "Renderer.h"

#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << " " << line << std::endl;
        return false;
    }
    return true;
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
    shader.Bind();
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::DrawSkybox(const Skybox& skybox, const glm::mat4& view, const glm::mat4& proj) const
{
    GLCall(glDepthFunc(GL_LEQUAL));

	skybox.Bind();

    glm::mat4 skyView = glm::mat4(glm::mat3(view));
    skybox.GetShader().SetUniformMat4f("u_View", skyView);
    skybox.GetShader().SetUniformMat4f("u_Proj", proj);

    GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.GetID()));
    skybox.GetShader().SetUniform1i("skybox", 0);

    skybox.GetVA().Bind();

    // Since Skybox has no IndexBuffer, we use glDrawArrays
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 36));

    GLCall(glDepthFunc(GL_LESS));
}

void Renderer::Clear() const
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));;
}