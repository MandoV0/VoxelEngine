#pragma once

#include "Chunk.h"
#include <memory>
#include <glm.hpp>
#include "../vendor/FastNoiseLite.h"

#include <functional>
#include "../Camera.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>

class Chunk;

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
	World(int seed);
	~World();

	Chunk* GetChunk(int cx, int cz);
	Chunk& CreateChunk(int cx, int cz);

	void UpdateChunksInRadius(int cx, int cz, int renderDistance);
	void GenerateChunk(int cx, int cz);

	void DropChunk(int cx, int cz);

	BlockType GetBlock(int wx, int wy, int wz);
	void SetBlock(int wx, int wy, int wz, BlockType type);

	void Render(Renderer& renderer, Shader& shader, Camera& camera);

	bool Raycast(glm::vec3 origin, glm::vec3 direction, float maxDistance, glm::ivec3& hitBlock, glm::ivec3& placeBlock);

	void RenderBlockOutline(Renderer& renderer, Shader& shader, int wx, int wy, int wz);

	void SpawnTree(int wx, int height, int wz);

	void GenerateTree(std::shared_ptr<Chunk> chunk, int x, int y, int z, int height); // Experimental

	float GetTreeNoise(int wx, int wz);

	static int WorldToChunk(int x);
	static int WorldToLocal(int x);

	static bool isInFOV2D(const glm::vec3& cameraPos, const glm::vec3& cameraForward, float fovDegrees, const glm::vec3& point)
	{
		glm::vec3 camForwardXZ = glm::normalize(glm::vec3(cameraForward.x, 0.0f, cameraForward.z));
		glm::vec3 toPointXZ = glm::normalize(glm::vec3(point.x - cameraPos.x, 0.0f, point.z - cameraPos.z));

		float cosAngle = glm::dot(camForwardXZ, toPointXZ);
		float halfFovRad = glm::radians(fovDegrees * 0.5f);
		float cosHalfFov = cos(halfFovRad);

		return cosAngle >= cosHalfFov;
	}

	void NotifyNeighborsOfNewChunk(int cx, int cz);
	void MarkChunkDirty(int cx, int cz);



private:	
	int m_Seed;
	FastNoiseLite m_Noise { m_Seed };
	std::unordered_map<glm::ivec2, std::shared_ptr<Chunk>> m_Chunks;

	// Raycasting cache
	int lastcx = INT_MIN;
	int lastcz = INT_MIN;
	Chunk* raycastChunk = nullptr;

	std::queue<glm::ivec2> m_ChunkGenQueue;

	// Thread Pool
	std::vector<std::thread> m_WorkerThreads;
	std::queue<std::function<void()>> m_JobQueue;
	std::mutex m_QueueMutex;
	std::condition_variable m_Condition;
	std::atomic<bool> m_ShutDownThreadPool{ false };

	void InitThreadPool(int numThreads = 4);
	void ShutdownThreadPool();
	void WorkerThreadLoop();
	void EnqueueJob(std::function<void()> job);
	

};