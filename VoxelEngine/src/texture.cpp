#include "texture.h"

#include "vendor/stb_image/stb_image.h"

#include <iostream>

Texture::Texture(const std::string& path)
	: m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr),
	m_Width(0), m_Height(0), m_BPP(0)
{
	stbi_set_flip_vertically_on_load(1);
	m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);

	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));


	if (m_LocalBuffer) {
		stbi_image_free(m_LocalBuffer);
	}
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int slot) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

unsigned int Texture::LoadCubemap(const std::string& path)
{
	unsigned int textureID;
	GLCall(glGenTextures(1, &textureID));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

	int width, height, nrChannels;

	stbi_set_flip_vertically_on_load(0);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	stbi_set_flip_vertically_on_load(1);

	if (data)
	{
		// 4x3 Layout
		int faceWidth = width / 4;
		int faceHeight = height / 3;

		struct { int x, y; } offsets[6] = {
			{ 2, 1 }, // Right
			{ 0, 1 }, // Left
			{ 1, 0 }, // Top
			{ 1, 2 }, // Bottom
			{ 1, 1 }, // Front
			{ 3, 1 }  // Back
		};

		unsigned char* faceData = new unsigned char[faceWidth * faceHeight * nrChannels];

		for (unsigned int i = 0; i < 6; i++)
		{
			for (int y = 0; y < faceHeight; y++)
			{
				int srcY = offsets[i].y * faceHeight + y;
				int srcX = offsets[i].x * faceWidth;

				memcpy(faceData + y * faceWidth * nrChannels,
					data + (srcY * width + srcX) * nrChannels,
					faceWidth * nrChannels);
			}

			GLenum format = GL_RGB;
			if (nrChannels == 4)
				format = GL_RGBA;
			else if (nrChannels == 1)
				format = GL_RED;

			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faceData));
		}
		delete[] faceData;
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Cubemap texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	return textureID;
}
