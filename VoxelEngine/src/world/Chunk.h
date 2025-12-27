#pragma once
#include <glm.hpp>
#include "../Shader.h"
#include "Block.h"
#include "../VertexArray.h"
#include "../IndexBuffer.h"

class Chunk
{
private:
	glm::ivec3 m_Position;
	VertexArray* m_VA;
	VertexBuffer* m_VB;
	IndexBuffer* m_IB;
	std::vector<Block> m_Blocks[16][16][16];

public:
	Chunk(glm::ivec3 position);
	
	void GenerateMesh();
	void Render(Shader& shader);
	void SetBlock(int x, int y, int z, BlockType blockType);
	void CreateBlock(std::vector<float>& vertices, std::vector<unsigned int>& indicies, int x, int y, int z);
	BlockType GetBlockType(int x, int y, int z);
};