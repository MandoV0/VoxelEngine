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
    //shader.Bind();
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

void Renderer::BeginGeometryPass(const GBuffer& gBuffer)
{
    m_CurrentGBuffer = &gBuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.m_FBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndGeometryPass() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::BeginLightingPass() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind GBuffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_CurrentGBuffer->m_PositionTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_CurrentGBuffer->m_NormalTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_CurrentGBuffer->m_AlbedoTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_CurrentGBuffer->m_MetallicTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_CurrentGBuffer->m_RoughnessTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_CurrentGBuffer->m_AOTex);
}

void Renderer::EndLightingPass() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::BlitDepthBuffer(const GBuffer& gBuffer, int width, int height)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::BeginShadowPass(const ShadowMap& shadowMap)
{
    shadowMap.Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    // Cull front faces for shadows to prevent peter panning (optional but often good)
    // glCullFace(GL_FRONT);
}

void Renderer::EndShadowPass(int viewportWidth, int viewportHeight) const
{
    // glCullFace(GL_BACK); // Reset culling
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewportWidth, viewportHeight);
}

void Renderer::RenderQuad() {
    if (m_QuadVAO == 0) {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &m_QuadVAO);
        glGenBuffers(1, &m_QuadVBO);

        glBindVertexArray(m_QuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void Renderer::Clear() const
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));;
}