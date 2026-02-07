#include "World.h"

#include "../VertexBufferLayout.h"

#include <algorithm>
#include <iostream>

World::World(int seed)
{
	m_Seed = seed;
	InitThreadPool();
}

World::~World()
{
	ShutdownThreadPool();
}

Chunk* World::GetChunk(int cx, int cz)
{
    glm::ivec2 coord { cx, cz };

    auto it = m_Chunks.find(coord);
    if (it == m_Chunks.end())
        return nullptr;

    return it->second.get();
}

Chunk& World::CreateChunk(int cx, int cz)
{
    glm::ivec2 coord { cx, cz };

    auto it = m_Chunks.find(coord);
    if (it != m_Chunks.end())
        return *it->second;

    auto chunk = std::make_unique<Chunk>(glm::ivec2(cx, cz));
    Chunk& ref = *chunk;

    m_Chunks.emplace(coord, std::move(chunk));
    return ref;
}

void World::UpdateChunksInRadius(int cx, int cz, int renderDistance)
{
	// Assume: We have the Camera Position in Chunk Coordinates (cx, cz)
	// We have the render distance in chunks
	// We only want to load all chunks in renderDistance around the camera meaning camera +- renderDistance in both directions.
    // So each frame? check if all chunks in that area exist, if not create and generate them.
	// For all other chunks just drop them from memory?????
    // Kinda slow

	for (int i = -renderDistance; i <= renderDistance; i++)
    {
        for (int j = -renderDistance; j <= renderDistance; j++)
        {
            int chunkX = cx + i;
            int chunkZ = cz + j;

            Chunk* chunk = GetChunk(chunkX, chunkZ);


            if (!chunk)
            {
                GenerateChunk(chunkX, chunkZ);
            }
            else if (chunk->IsTerrainGenerated()) {
				// Only update chunks when dirty
				chunk->Update(this);
            }
        }
    }
}

void World::GenerateChunk(int cx, int cz) {
    auto chunkPtr = std::make_shared<Chunk>(glm::ivec2(cx, cz));
    m_Chunks[glm::ivec2(cx, cz)] = chunkPtr;

    EnqueueJob([this, cx, cz, chunkPtr]() {
        constexpr int CHUNK_SIZE = 16;
        constexpr int CHUNK_HEIGHT = 128;
        constexpr int SEA_LEVEL = 62;
        constexpr int MIN_HEIGHT = 45;
        constexpr int MAX_HEIGHT = 95;

        // Base terrain. smooth rolling hills
        FastNoiseLite baseNoise(m_Seed);
        baseNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        baseNoise.SetFrequency(0.004f);
        baseNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
        baseNoise.SetFractalOctaves(4);
        baseNoise.SetFractalLacunarity(2.0f);
        baseNoise.SetFractalGain(0.5f);

        // Mountains
        FastNoiseLite mountainNoise(m_Seed + 1);
        mountainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        mountainNoise.SetFrequency(0.008f);
		mountainNoise.SetFractalGain(0.4f);
        mountainNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
        mountainNoise.SetFractalOctaves(3);

        // Mountain mask.
        FastNoiseLite mountainMask(m_Seed + 2);
        mountainMask.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        mountainMask.SetFrequency(0.002f);

        // TODO: Both Mountains passes are extremely mild.

        // Tree Noise
        FastNoiseLite treeDensityNoise(m_Seed + 3);
        treeDensityNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        treeDensityNoise.SetFrequency(0.05f);

        int heightMap[CHUNK_SIZE][CHUNK_SIZE];

        // HEIGHT PASS
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int worldX = cx * CHUNK_SIZE + x;
                int worldZ = cz * CHUNK_SIZE + z;

                float base = baseNoise.GetNoise((float)worldX, (float)worldZ);
                base = (base + 1.0f) * 0.5f;

                float mountain = std::abs(mountainNoise.GetNoise((float)worldX, (float)worldZ));
                mountain = std::pow(mountain * 3, 1.8f);

                float mask = mountainMask.GetNoise((float)worldX, (float)worldZ);
                mask = (mask + 1.0f) * 0.5f;
                mask = std::pow(mask, 2.5f);

                float finalHeight = base * 0.6f + mountain * mask * 0.7f;

                int height = MIN_HEIGHT + (int)(finalHeight * (MAX_HEIGHT - MIN_HEIGHT));
                height = std::clamp(height, 1, CHUNK_HEIGHT - 1);

                heightMap[x][z] = height;
            }
        }

        // BLOCK PASS
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int height = heightMap[x][z];

                for (int y = 0; y <= height; y++) {
                    BlockType blockType;

                    if (y == height) {
                        if (height < SEA_LEVEL - 2) {
                            blockType = BlockType::DIRT;
                        }
                        else if (height > 85) {
                            blockType = BlockType::STONE;
                        }
                        else {
                            blockType = BlockType::GRASS;
                        }
                    }
                    else if (y > height - 4) {
                        blockType = BlockType::DIRT;
                    }
                    else {
                        blockType = BlockType::STONE;
                    }

                    chunkPtr->SetBlock(x, y, z, blockType);
                }

                if (height < SEA_LEVEL) {
                    for (int y = height + 1; y <= SEA_LEVEL; y++) {
                        chunkPtr->SetBlock(x, y, z, BlockType::WATER);
                    }
                }
            }
        }

        // TREE PASS
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int worldX = cx * CHUNK_SIZE + x;
                int worldZ = cz * CHUNK_SIZE + z;
                int height = heightMap[x][z];

				// Spawn Trees only on Grass and above sea level, and below a certain height to avoid mountain tops
                if (height <= SEA_LEVEL || height > 80) continue;
                if (chunkPtr->GetBlockType(x, height, z) != BlockType::GRASS) continue;

                // Tree density check
                float density = treeDensityNoise.GetNoise((float)worldX, (float)worldZ);
                density = (density + 1.0f) * 0.5f;  // 0 - 1

                // Random tree placement using world coordinates as seed. Should probably explore blue noise here for good tree placement. TODO
                unsigned int hash = (worldX * 374761393) + (worldZ * 668265263);
                float random = (float)(hash % 10000) / 10000.0f;

                // Spawn tree?
                float spawnChance = 0.01f + density * 0.01f;  // 5-20% chance
                if (random < spawnChance) {
                    // Choose tree type
                    float treeTypeRand = (float)((hash >> 16) % 100) / 100.0f;

                    GenerateTree(chunkPtr, x, height + 1, z, 4 + (hash % 3));
                }
            }
        }

        chunkPtr->SetTerrainGenerated(true);
        chunkPtr->SetIsFullyLoaded(true);
        chunkPtr->SetIsDirty(true);

        NotifyNeighborsOfNewChunk(cx, cz);
        });
}

// Helper function for tree generation
void World::GenerateTree(std::shared_ptr<Chunk> chunk, int x, int y, int z, int height) {
    constexpr int CHUNK_SIZE = 16;

    // Trunk
    for (int i = 0; i < height; i++) {
        if (y + i < 128) {
            chunk->SetBlock(x, y + i, z, BlockType::WOOD);
        }
    }

    // Leaves - 3 layers
    int topY = y + height;

    // Top layer (cross shape)
    if (topY < 128) {
        chunk->SetBlock(x, topY, z, BlockType::LEAF);
        if (x > 0) chunk->SetBlock(x - 1, topY, z, BlockType::LEAF);
        if (x < CHUNK_SIZE - 1) chunk->SetBlock(x + 1, topY, z, BlockType::LEAF);
        if (z > 0) chunk->SetBlock(x, topY, z - 1, BlockType::LEAF);
        if (z < CHUNK_SIZE - 1) chunk->SetBlock(x, topY, z + 1, BlockType::LEAF);
    }

    // Middle and bottom layers (larger)
    for (int layerOffset = 1; layerOffset <= 2; layerOffset++) {
        int layerY = topY - layerOffset;
        if (layerY < 0 || layerY >= 128) continue;

        int radius = (layerOffset == 1) ? 2 : 2;

        for (int dx = -radius; dx <= radius; dx++) {
            for (int dz = -radius; dz <= radius; dz++) {
                int lx = x + dx;
                int lz = z + dz;

                // Skip trunk
                if (dx == 0 && dz == 0) continue;

                // Skip corners
                if (std::abs(dx) == radius && std::abs(dz) == radius) {
                    if (rand() % 2 == 0) continue;  // 50% chance to skip corners
                }

                // Check bounds
                if (lx >= 0 && lx < CHUNK_SIZE && lz >= 0 && lz < CHUNK_SIZE) {
                    BlockType existing = chunk->GetBlockType(lx, layerY, lz);
                    if (existing == BlockType::AIR) {
                        chunk->SetBlock(lx, layerY, lz, BlockType::LEAF);
                    }
                }
            }
        }
    }
}

void World::NotifyNeighborsOfNewChunk(int cx, int cz) {
	// Mark all 4 Neighbors as dirty so they update and re render the faces towards this new chunk
    Chunk* leftN = GetChunk(cx - 1, cz);
    Chunk* rightN = GetChunk(cx + 1, cz);
    Chunk* backN = GetChunk(cx, cz - 1);
    Chunk* frontN = GetChunk(cx, cz + 1);

    if (leftN && leftN->GetIsFullyLoaded() && leftN->IsTerrainGenerated()) {
        leftN->SetIsDirty(true);
    }
    if (rightN && rightN->GetIsFullyLoaded() && rightN->IsTerrainGenerated()) {
        rightN->SetIsDirty(true);
    }
    if (backN && backN->GetIsFullyLoaded() && backN->IsTerrainGenerated()) {
        backN->SetIsDirty(true);
    }
    if (frontN && frontN->GetIsFullyLoaded() && frontN->IsTerrainGenerated()) {
        frontN->SetIsDirty(true);
    }
}

void World::MarkChunkDirty(int cx, int cz) {
    Chunk* chunk = GetChunk(cx, cz);
    if (chunk && chunk->GetIsFullyLoaded()) {
        chunk->SetIsDirty(true);;
    }
}

void World::DropChunk(int cx, int cz)
{
    glm::ivec2 coord { cx, cz };
	m_Chunks.erase(coord);
}

BlockType World::GetBlock(int wx, int wy, int wz)
{
    int cx = WorldToChunk(wx);
    int cz = WorldToChunk(wz);

    Chunk* chunk = GetChunk(cx, cz);
    if (!chunk) return BlockType::AIR;

    return chunk->GetBlockType(
        WorldToLocal(wx),
        wy,
        WorldToLocal(wz)
    );
}

void World::SetBlock(int wx, int wy, int wz, BlockType type)
{
    int cx = WorldToChunk(wx);
    int cz = WorldToChunk(wz);

    Chunk& chunk = CreateChunk(cx, cz);

    chunk.SetBlock(
        WorldToLocal(wx),
        wy,
        WorldToLocal(wz),
        type
    );
}

void World::Render(Renderer& renderer, Shader& shader, Camera& camera)
{
    float maxViewDistance = 12 * Chunk::WIDTH; // e.g., 12 chunks away
	bool frustumCulling = false;
        
    for (auto& [coord, chunk] : m_Chunks)
    {

        // Broken when high up in the air????
        if (frustumCulling)
        {
            glm::vec3 camPosXZ = glm::vec3(camera.GetPosition().x, 0, camera.GetPosition().z);
            glm::vec3 camForwardXZ = glm::normalize(glm::vec3(camera.GetFront().x, 0, camera.GetFront().z));

            float chunkCenterX = coord.x * Chunk::WIDTH + Chunk::WIDTH * 0.5f;
            float chunkCenterZ = coord.y * Chunk::WIDTH + Chunk::WIDTH * 0.5f;

            float minX = chunkCenterX - Chunk::WIDTH * 0.5f;
            float maxX = chunkCenterX + Chunk::WIDTH * 0.5f;
            float minZ = chunkCenterZ - Chunk::WIDTH * 0.5f;
            float maxZ = chunkCenterZ + Chunk::WIDTH * 0.5f;

            // Closest point method
		    // Instead of checking distance to all 4 corners, we can find the closest point of the chunk that can enter the camera.
            float closestX = glm::clamp(camPosXZ.x, minX, maxX);
            float closestZ = glm::clamp(camPosXZ.z, minZ, maxZ);
            glm::vec3 closestPoint = glm::vec3(closestX, 0, closestZ);

            float distSq = glm::distance(camPosXZ, closestPoint);

		    // The distance check is to avoid the current chunk we are being in not being rendered.
            if (distSq > maxViewDistance && !isInFOV2D(camPosXZ, camForwardXZ, 85.f, closestPoint))
            {
                continue; // Skip chunk
            }
        }

        chunk->Render(renderer, shader);
    }
}

bool World::Raycast(glm::vec3 origin, glm::vec3 direction, float maxDistance, glm::ivec3& hitBlock, glm::ivec3& placeBlock)
{
    // The DDA algorithm works on a grid where boundaries are at integers (0, 1, 2).
    // But our blocks are centered at integers (0,0,0), so the boundaries are at -0.5, 0.5, so the indicies.
    // By adding 0.5 to the origin, we shift the world so that block boundaries align with the integers so the algoritm works.
    glm::vec3 ddaOrigin = origin + 0.5f;

    // Convert to block coordinates
    int x = static_cast<int>(floor(ddaOrigin.x));
    int y = static_cast<int>(floor(ddaOrigin.y));
    int z = static_cast<int>(floor(ddaOrigin.z));

    int stepX = (direction.x > 0) ? 1 : -1; // signum
    int stepY = (direction.y > 0) ? 1 : -1;
    int stepZ = (direction.z > 0) ? 1 : -1;

    float tDeltaX = (direction.x != 0.0f) ? fabs(1.0f / direction.x) : std::numeric_limits<float>::max();
    float tDeltaY = (direction.y != 0.0f) ? fabs(1.0f / direction.y) : std::numeric_limits<float>::max();
    float tDeltaZ = (direction.z != 0.0f) ? fabs(1.0f / direction.z) : std::numeric_limits<float>::max();

    // Distance to first voxel boundary
    float tMaxX = (direction.x > 0) ? (floor(ddaOrigin.x + 1.0f) - ddaOrigin.x) * tDeltaX
        : (ddaOrigin.x - floor(ddaOrigin.x)) * tDeltaX;
    float tMaxY = (direction.y > 0) ? (floor(ddaOrigin.y + 1.0f) - ddaOrigin.y) * tDeltaY
        : (ddaOrigin.y - floor(ddaOrigin.y)) * tDeltaY;
    float tMaxZ = (direction.z > 0) ? (floor(ddaOrigin.z + 1.0f) - ddaOrigin.z) * tDeltaZ
        : (ddaOrigin.z - floor(ddaOrigin.z)) * tDeltaZ;

    float distanceTraveled = 0.0f;
    int lastX = x;
    int lastY = y;
    int lastZ = z;

	// Cache chunks to avoid repeated lookups

    while (distanceTraveled <= maxDistance)
    {
		int cx = WorldToChunk(x);
		int cz = WorldToChunk(z);

		// Hash map lookup can be expensive, so we cache the last chunk we accessed.
        if (!raycastChunk || cx != lastcx || cz != lastcz)
        {
            raycastChunk = GetChunk(cx, cz);
            lastcx = cx;
			lastcz = cz;
        }

        if (raycastChunk)
        {
            BlockType type = raycastChunk->GetBlockType(WorldToLocal(x), y, WorldToLocal(z));

            if (type != BlockType::AIR)
            {
                hitBlock = glm::ivec3(x, y, z);
				placeBlock = glm::ivec3(lastX, lastY, lastZ); // For easier Block placement.
                return true;
            }
        }

        lastX = x;
        lastY = y;
        lastZ = z;

        // Next Voxel step
        if (tMaxX < tMaxY && tMaxX < tMaxZ)
        {
            x += stepX;
            distanceTraveled = tMaxX;
            tMaxX += tDeltaX;
        }
        else if (tMaxY < tMaxZ)
        {
            y += stepY;
            distanceTraveled = tMaxY;
            tMaxY += tDeltaY;
        }
        else
        {
            z += stepZ;
            distanceTraveled = tMaxZ;
            tMaxZ += tDeltaZ;
        }
    }

    return false;
}

void World::RenderBlockOutline(Renderer& renderer, Shader& shader, int wx, int wy, int wz)
{
    glLineWidth(2.0f);

    float x = wx;
    float y = wy;
    float z = wz;
    float offset = 0.505f;

    // 8 Corners of the cube/voxel
    glm::vec3 c0(x - offset, y - offset, z - offset);
    glm::vec3 c1(x + offset, y - offset, z - offset);
    glm::vec3 c2(x + offset, y + offset, z - offset);
    glm::vec3 c3(x - offset, y + offset, z - offset);
    glm::vec3 c4(x - offset, y - offset, z + offset);
    glm::vec3 c5(x + offset, y - offset, z + offset);
    glm::vec3 c6(x + offset, y + offset, z + offset);
    glm::vec3 c7(x - offset, y + offset, z + offset);

    // 12 Edges (pairs of vertices)
    float outlineVertices[] = {
        c0.x, c0.y, c0.z, 0.0f, 0.0f,  c1.x, c1.y, c1.z, 0.0f, 0.0f, // Bottom Back
        c1.x, c1.y, c1.z, 0.0f, 0.0f,  c2.x, c2.y, c2.z, 0.0f, 0.0f, // Right Back
        c2.x, c2.y, c2.z, 0.0f, 0.0f,  c3.x, c3.y, c3.z, 0.0f, 0.0f, // Top Back
        c3.x, c3.y, c3.z, 0.0f, 0.0f,  c0.x, c0.y, c0.z, 0.0f, 0.0f, // Left Back

        c4.x, c4.y, c4.z, 0.0f, 0.0f,  c5.x, c5.y, c5.z, 0.0f, 0.0f, // Bottom Front
        c5.x, c5.y, c5.z, 0.0f, 0.0f,  c6.x, c6.y, c6.z, 0.0f, 0.0f, // Right Front
        c6.x, c6.y, c6.z, 0.0f, 0.0f,  c7.x, c7.y, c7.z, 0.0f, 0.0f, // Top Front
        c7.x, c7.y, c7.z, 0.0f, 0.0f,  c4.x, c4.y, c4.z, 0.0f, 0.0f, // Left Front

        c0.x, c0.y, c0.z, 0.0f, 0.0f,  c4.x, c4.y, c4.z, 0.0f, 0.0f, // Bottom Left
        c1.x, c1.y, c1.z, 0.0f, 0.0f,  c5.x, c5.y, c5.z, 0.0f, 0.0f, // Bottom Right
        c2.x, c2.y, c2.z, 0.0f, 0.0f,  c6.x, c6.y, c6.z, 0.0f, 0.0f, // Top Right
        c3.x, c3.y, c3.z, 0.0f, 0.0f,  c7.x, c7.y, c7.z, 0.0f, 0.0f  // Top Left
    };

    unsigned int outlineIndices[24];
    for (int i = 0; i < 24; i++) outlineIndices[i] = i;

    VertexArray outlineVA;
    VertexBuffer outlineVB(outlineVertices, sizeof(outlineVertices));

    VertexBufferLayout layout;
    layout.Push<float>(3); // X, Y, Z
    layout.Push<float>(2); // U, V
    outlineVA.AddBuffer(outlineVB, layout);

    IndexBuffer outlineIB(outlineIndices, 24);

    outlineVA.Bind();
    outlineIB.Bind();
    shader.Bind();
    glDrawElements(GL_LINES, outlineIB.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void World::SpawnTree(int worldX, int worldY, int worldZ) {
    int trunkHeight = 4 + (rand() % 3);

    for (int i = 0; i < trunkHeight; i++) {
        SetBlock(worldX, worldY + i, worldZ, BlockType::WOOD);
    }

    // Generate the Layers
    // From top to bottom
    for (int ly = -2; ly <= 1; ly++) {
        int yPos = worldY + trunkHeight + ly;
        int radius = (ly > -1) ? 1 : 2;

        for (int lx = -radius; lx <= radius; lx++) {
            for (int lz = -radius; lz <= radius; lz++) {

                if (lx == 0 && lz == 0 && ly < 0) continue;

                // Corner Cutting
                if (abs(lx) == radius && abs(lz) == radius) {
                    if (radius > 1 || (rand() % 2 == 0)) continue;
                }

                if (GetBlock(worldX + lx, yPos, worldZ + lz) == BlockType::AIR) {
                    SetBlock(worldX + lx, yPos, worldZ + lz, BlockType::LEAF);
                }
            }
        }
    }
}

float World::GetTreeNoise(int wx, int wz)
{
	// Replace this with a good noise function
    unsigned int seed = 12345;
    unsigned int h = seed ^ (wx * 1327144033) ^ (wz * 3575866297);
    return (float)(h % 1000) / 1000.0f;
}

int World::WorldToChunk(int x)
{
    return floor((float)x / 16);
}

int World::WorldToLocal(int x)
{
    int r = x % 16;
    return r < 0 ? r + 16 : r;
}

void World::InitThreadPool(int numThreads)
{
	for (int i = 0; i < numThreads; i++)
    {
        m_WorkerThreads.emplace_back(&World::WorkerThreadLoop, this);
    }
}

void World::WorkerThreadLoop()
{
    while (true)
    {
        std::function<void()> job;
        {
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_Condition.wait(lock, [this]
            {
                    return m_ShutDownThreadPool || !m_JobQueue.empty();
            });

            if (m_ShutDownThreadPool && m_JobQueue.empty())
				return;

			job = std::move(m_JobQueue.front());
			m_JobQueue.pop();
        }
        job();
	}
}

void World::ShutdownThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_ShutDownThreadPool = true;
    }
    m_Condition.notify_all();

    for (auto& thread : m_WorkerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void World::EnqueueJob(std::function<void()> job)
{
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_JobQueue.push(std::move(job));
    }
    m_Condition.notify_one();
}