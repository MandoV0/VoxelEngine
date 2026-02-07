#include <GL/glew.h>
#include <iostream>

/*
* Geometry Buffer for Deferred Renderer
*/
class GBuffer {
private:
    unsigned int m_FBO;
    unsigned int m_PositionTex;    // RGB: world position
    unsigned int m_NormalTex;      // RGB: world normal
    unsigned int m_AlbedoTex;      // RGB: Color
    unsigned int m_PBRTex;         // R: metallic, G: roughness, B: AO Texture Packing
    unsigned int m_DepthTex;       // Depth + stencil textures

public:
    void Create(int width, int height) {
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // Position buffer (16F for precision)
        glGenTextures(1, &m_PositionTex);
        glBindTexture(GL_TEXTURE_2D, m_PositionTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PositionTex, 0);

        // Normal buffer
        glGenTextures(1, &m_NormalTex);
        glBindTexture(GL_TEXTURE_2D, m_NormalTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_NormalTex, 0);

        // Albedo buffer
        glGenTextures(1, &m_AlbedoTex);
        glBindTexture(GL_TEXTURE_2D, m_AlbedoTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_AlbedoTex, 0);

        // TODO: PBR parameters (metallic, roughness, AO)
        glGenTextures(1, &m_PBRTex);
        glBindTexture(GL_TEXTURE_2D, m_PBRTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,
            GL_TEXTURE_2D, m_PBRTex, 0);

        // 4 color attachments
        unsigned int attachments[4] = {
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
        };
        glDrawBuffers(4, attachments);

        // Depth buffer
        glGenTextures(1, &m_DepthTex);
        glBindTexture(GL_TEXTURE_2D, m_DepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthTex, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "GBuffer not complete!!!!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};
