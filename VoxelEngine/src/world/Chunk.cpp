#include "Chunk.h"

void Chunk::GenerateMesh()
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			BlockType blockType = GetBlockType(i, j, 0);

			CreateBlock(vertices, indices, i, j, 0);
		}
	}

}

void Chunk::CreateBlock(std::vector<float>& vertices, std::vector<unsigned int>& indicies, int x, int y, int z)
{
	float vertices[] = {
		// front
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

		// back
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

		 // left
		 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

		 // right
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

		 // top
		 -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

		 // bottom
		 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		  0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		  0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
		 -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
	};


	unsigned int indices[] = {
		0,  1,  2,   2,  3,  0,		// Front
		4,  5,  6,   6,  7,  4,		// Back
		8,  9, 10,  10, 11,  8,		// Left
		12, 15, 14,  14, 13, 12,	// Right
		16, 17, 18,  18, 19, 16,	// Top
		20, 21, 22,  22, 23, 20		// Bottom
	};
}
