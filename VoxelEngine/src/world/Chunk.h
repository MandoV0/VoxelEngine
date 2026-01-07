#pragma once
#include <glm.hpp>
#include "../Shader.h"
#include "Block.h"
#include "../VertexArray.h"
#include "../IndexBuffer.h"
#include "../Renderer.h"

enum class FaceDirection {
	FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM
};

class Chunk
{

static constexpr int WIDTH = 16;
static constexpr int HEIGHT = 128;

private:
	/*
	* Chunk Position in chunk space, NOT WORLD SPACE
	* Example (0, 0) = chunk at world (0...15, 0....15)
	*/
	glm::ivec2 m_ChunkPosition;
	VertexArray* m_VA;
	VertexBuffer* m_VB;
	IndexBuffer* m_IB;
	Block m_Blocks[WIDTH][HEIGHT][WIDTH];

	bool m_HasSelectedBlock;
	glm::ivec3 m_SelectedBlock;

	bool IsAir(int x, int y, int z);
	void AddFace(FaceDirection dir, int x, int y, int z, std::vector<float>& vertices, std::vector<unsigned int>& indices);

public:
	Chunk(glm::ivec2 position);
	~Chunk();
	
	void GenerateMesh();
	void Render(Renderer& renderer, Shader& shader);

	void SetBlock(int x, int y, int z, BlockType blockType);
	void CreateBlock(std::vector<float>& vertices, std::vector<unsigned int>& indicies, int x, int y, int z);

	BlockType GetBlockType(int x, int y, int z);

	void SetSelectedBlock(bool hasBlock, glm::ivec3 position);
};