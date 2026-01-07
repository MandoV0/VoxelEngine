#include "Framebuffer.h"

Framebuffer::Framebuffer(int width, int height)
	: m_Width(width), m_Height(height), m_RendererID(0), m_DepthAttachment(0)
{
	glGenFramebuffers(1, &m_RendererID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &m_RendererID);
	for (auto id : m_ColorAttachments)
		glDeleteTextures(1, &id);
	glDeleteRenderbuffers(1, &m_DepthAttachment);
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	glViewport(0, 0, m_Width, m_Height);
}

void Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::AddColorAttachment(GLint internalFormat, GLenum format, GLenum type)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, type, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_ColorAttachments.size(), GL_TEXTURE_2D, textureID, 0);
	
	m_ColorAttachments.push_back(textureID);
}

void Framebuffer::AddDepthAttachment()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

	glGenRenderbuffers(1, &m_DepthAttachment);
	glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachment);
}

void Framebuffer::CheckStatus()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

	if (m_ColorAttachments.size() > 0)
	{
		std::vector<unsigned int> attachments;
		for (int i = 0; i < m_ColorAttachments.size(); i++)
			attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		glDrawBuffers(attachments.size(), attachments.data());
	}
	else
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::BlitDepthToDefault(unsigned int scrWidth, unsigned int scrHeight)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
	glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, scrWidth, scrHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}