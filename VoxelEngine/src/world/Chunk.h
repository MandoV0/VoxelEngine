#pragma once
#include <glm.hpp>
#include "../Shader.h"
#include "Block.h"
#include "../VertexArray.h"
#include "../IndexBuffer.h"
#include "../Renderer.h"

class Chunk
{
private:
	glm::ivec3 m_Position;
	VertexArray* m_VA;
	VertexBuffer* m_VB;
	IndexBuffer* m_IB;
	Block m_Blocks[16][16][16];

	bool m_HasSelectedBlock;
	glm::ivec3 m_SelectedBlock;
public:
	Chunk(glm::ivec3 position);
	
	void GenerateMesh();
	void Render(Renderer& renderer, Shader& shader);
	void SetBlock(int x, int y, int z, BlockType blockType);
	void CreateBlock(std::vector<float>& vertices, std::vector<unsigned int>& indicies, int x, int y, int z);
	BlockType GetBlockType(int x, int y, int z);
	bool Raycast(glm::vec3 origin, glm::vec3 direction, float maxDistance, glm::ivec3& hitBlock, glm::ivec3& placeBlock);

	void SetSelectedBlock(bool hasBlock, glm::ivec3 position);
	void RenderBlockOutline(Renderer& renderer, Shader& shader);
};