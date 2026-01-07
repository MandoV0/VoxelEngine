#pragma once
#include <glm.hpp>
#include <vector>
#include <mutex>
#include <atomic>
#include "../Shader.h"
#include "Block.h"
#include "../VertexArray.h"
#include "../VertexBuffer.h"
#include "../IndexBuffer.h"
#include "../Renderer.h"

class Chunk
{
public:
	static constexpr int WIDTH = 16;
	static constexpr int HEIGHT = 128;

	struct ChunkData {
		Block blocks[WIDTH][HEIGHT][WIDTH];
	};

private:
	/*
	* Chunk Position in chunk space, NOT WORLD SPACE
	* Example (0, 0) = chunk at world (0...15, 0....15)
	*/
	glm::ivec2 m_ChunkPosition;

	VertexArray* m_VA;
	VertexBuffer* m_VB;
	IndexBuffer* m_IB;

	ChunkData m_Blocks;

	bool m_HasSelectedBlock;
	glm::ivec3 m_SelectedBlock;

	// Threading support
	std::mutex m_MeshMutex;
	std::atomic<bool> m_HasNewMesh{ false };
	std::atomic<bool> m_IsDirty{ false };
	std::atomic<bool> m_IsGenerating{ false };

	std::vector<float> m_IntermediateVertices;
	std::vector<unsigned int> m_IntermediateIndices;

	// Helper methods
	bool IsAir(int x, int y, int z);
	static bool IsSolid(BlockType type);
	static BlockType GetBlockTypeFromData(const ChunkData& data, int x, int y, int z);

	static void CreateBlockWorker(const ChunkData& data, glm::ivec2 chunkPos,
		std::vector<float>& vertices,
		std::vector<unsigned int>& indices,
		int x, int y, int z);

	static void GenerateMeshWorker(Chunk* chunk, ChunkData data, glm::ivec2 position);

public:
	Chunk(glm::ivec2 position);
	~Chunk();

	void Update();
	void Render(Renderer& renderer, Shader& shader);
	void SetBlock(int x, int y, int z, BlockType blockType);
	BlockType GetBlockType(int x, int y, int z);
	void SetSelectedBlock(bool hasBlock, glm::ivec3 position);

	static float GetLightLevelAt(int x, int y, int z, const ChunkData& data);
};