#pragma once
#include <glm.hpp>
#include <vector>
#include <mutex>
#include <atomic>
#include "../Shader.h"
#include "Block.h"
#include <array>
#include "../VertexArray.h"
#include "../VertexBuffer.h"
#include "../IndexBuffer.h"
#include "../Renderer.h"

class World;

struct Vertex {
	float x, y, z;
	float u, v;
	float ao;
	float light;
};

enum class RenderLayer {
	SOLID = 0,
	CUTOUT = 1,
	TRANSLUCENT = 2
};

class Chunk
{
public:
	static constexpr int WIDTH = 16;
	static constexpr int HEIGHT = 128;

	struct ChunkData {
		Block blocks[WIDTH][HEIGHT][WIDTH];
	};

	// To know if neighboring blocks are solid, we create a padded version of ChunkData so we avoid rendering these faces unnecessarily.
	struct PaddedChunkData {
		Block blocks[WIDTH + 2][HEIGHT][WIDTH + 2]; // +2 So we have a 1 block padding on each side in X and Z
	};

private:
	/*
	* Chunk Position in chunk space, NOT WORLD SPACE
	* Example (0, 0) = chunk at world (0...15, 0....15)
	*/
	glm::ivec2 m_ChunkPosition;

	// For Solid Meshes
	VertexArray* m_VA;
	VertexBuffer* m_VB;
	IndexBuffer* m_IB;

	// For Water Meshes
	VertexArray* m_WaterVA;
	VertexBuffer* m_WaterVB;
	IndexBuffer* m_WaterIB;

	ChunkData m_Blocks;
	PaddedChunkData m_PaddedBlocks;

	bool m_HasSelectedBlock;
	glm::ivec3 m_SelectedBlock;

	// Threading support
	std::mutex m_MeshMutex;
	std::atomic<bool> m_HasNewMesh{ false };
	std::atomic<bool> m_IsDirty{ false };
	std::atomic<bool> m_IsGenerating{ false };
	bool m_IsTerrainGenerated{ false };

	std::vector<Vertex> m_IntermediateVertices;
	std::vector<unsigned int> m_IntermediateIndices;

	std::vector<Vertex> m_IntermediateWaterVertices;
	std::vector<unsigned int> m_IntermediateWaterIndices;
	bool m_HasNewWaterMesh = false;

	bool m_isFullyLoaded = false;

	// Helper methods
	bool IsAir(int x, int y, int z);
	static bool IsSolid(BlockType type);
	static BlockType GetBlockTypeFromData(const ChunkData& data, int x, int y, int z);
	static BlockType GetBlockTypeFromData(const PaddedChunkData& data, int x, int y, int z);

	static void CreateBlockWorker(const ChunkData& data, glm::ivec2 chunkPos,
		std::vector<Vertex>& vertices,
		std::vector<unsigned int>& indices,
		int x, int y, int z);

	static void CreateBlockWorker(const PaddedChunkData& data, glm::ivec2 chunkPos,
		std::vector<Vertex>& vertices,
		std::vector<unsigned int>& indices,
		int x, int y, int z);

	static void GenerateMeshWorker(Chunk* chunk, const PaddedChunkData data, glm::ivec2 position);

public:
	Chunk(glm::ivec2 position);
	~Chunk();

	void Update(World* world);

	void Render(Renderer& renderer, Shader& shader, int layer);

	void SetBlock(int x, int y, int z, BlockType blockType);
	BlockType GetBlockType(int x, int y, int z);
	void SetSelectedBlock(bool hasBlock, glm::ivec3 position);

	bool IsTerrainGenerated() const { return m_IsTerrainGenerated; }
	void SetTerrainGenerated(bool generated) { m_IsTerrainGenerated = generated; }

	static float GetLightLevelAt(int x, int y, int z, const ChunkData& data);

	bool GetIsFullyLoaded() const { return m_isFullyLoaded; }
	void SetIsFullyLoaded(bool loaded) { m_isFullyLoaded = loaded; }

	void SetIsDirty(bool dirty) { m_IsDirty = dirty; }
};