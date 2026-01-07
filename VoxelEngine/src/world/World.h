#pragma once

#include "Chunk.h"
#include <memory>
#include <glm.hpp>
#include "../vendor/FastNoiseLite.h"

#include <functional>

// ivec2 hash function for unordered_map, i cant get glms hash to work for some reason
namespace std {
	template<>
	struct hash<glm::ivec2> {
		size_t operator()(const glm::ivec2& v) const {
			size_t h1 = hash<int>{}(v.x);
			size_t h2 = hash<int>{}(v.y);
			
			return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
		}
	};
}

class World
{
public:
	World(int seed) : m_Seed(seed) {}

	Chunk* GetChunk(int cx, int cz);
	Chunk& CreateChunk(int cx, int cz);

	void GenerateChunk(int cx, int cz);

	BlockType GetBlock(int wx, int wy, int wz);
	void SetBlock(int wx, int wy, int wz, BlockType type);

	void Render(Renderer& renderer, Shader& shader);

	bool Raycast(glm::vec3 origin, glm::vec3 direction, float maxDistance, glm::ivec3& hitBlock, glm::ivec3& placeBlock);

	void RenderBlockOutline(Renderer& renderer, Shader& shader, int wx, int wy, int wz);

	static int WorldToChunk(int x);
	static int WorldToLocal(int x);

private:	
	int m_Seed;
	FastNoiseLite m_Noise { m_Seed };
	std::unordered_map<glm::ivec2, std::unique_ptr<Chunk> > m_Chunks;
};