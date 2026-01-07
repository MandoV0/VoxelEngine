#pragma once
#include <GL/glew.h>
#include <iostream>
#include <vector>

class Framebuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_DepthAttachment;
	std::vector<unsigned int> m_ColorAttachments;
	int m_Width, m_Height;

public:
	Framebuffer(int width, int height);
	~Framebuffer();

	void Bind();
	void Unbind();

	void AddColorAttachment(GLint internalFormat, GLenum format, GLenum type);
	void AddDepthAttachment();

	void CheckStatus();

	unsigned int GetColorAttachment(int index) const { return m_ColorAttachments[index]; }
	unsigned int GetDepthAttachment() const { return m_DepthAttachment; }
	unsigned int GetRendererID() const { return m_RendererID; }

	void BlitDepthToDefault(unsigned int scrWidth, unsigned int scrHeight);

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
};
