#include "World.h"

#include "../VertexBufferLayout.h"

#include <algorithm>
#include <iostream>

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

            if (!chunk || !chunk->IsTerrainGenerated())
            {
                GenerateChunk(chunkX, chunkZ);
            }
        }
    }
}

void World::GenerateChunk(int cx, int cz)
{
    Chunk& chunk = CreateChunk(cx, cz);

	m_Noise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
	m_Noise.SetFrequency(0.01f);
    m_Noise.SetFractalLacunarity(2.0f);
    m_Noise.SetFractalGain(0.5f);

    constexpr int CHUNK_SIZE = 16;
    constexpr int CHUNK_HEIGHT = 128;
    constexpr int MAX_HEIGHT = 64;
    constexpr int MIN_HEIGHT = 40;
    constexpr int SEA_LEVEL = 48;

    int heightMap[CHUNK_SIZE][CHUNK_SIZE];

    // Height
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int worldX = cx * CHUNK_SIZE + x;
            int worldZ = cz * CHUNK_SIZE + z;

            float noise = m_Noise.GetNoise((float)worldX, (float)worldZ);
            float n01 = (noise + 1.0f) * 0.5f;

            int height = MIN_HEIGHT + (int)(n01 * (MAX_HEIGHT - MIN_HEIGHT));

            float weirdness = std::fabs(noise);
            float pAndV = 1.0f - std::fabs((3.0f * weirdness) - 2.0f);
            pAndV = std::clamp(pAndV, 0.0f, 1.0f);

            height += (int)(pAndV * 15);
            height = std::clamp(height, 1, CHUNK_HEIGHT - 1);

            heightMap[x][z] = height;
        }
    }

    // Terrain
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int height = heightMap[x][z];

            for (int y = 0; y <= height; y++)
            {
                if (y == height)
                    chunk.SetBlock(x, y, z, BlockType::GRASS);
                else if (y > height - 4)
                    chunk.SetBlock(x, y, z, BlockType::DIRT);
                else
                    chunk.SetBlock(x, y, z, BlockType::STONE);
            }
        }
    }

    // Water
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int height = heightMap[x][z];

            for (int y = height + 1; y <= SEA_LEVEL; y++)
                chunk.SetBlock(x, y, z, BlockType::WATER);
        }
    }

    // Tree pass
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int worldX = cx * CHUNK_SIZE + x;
            int worldZ = cz * CHUNK_SIZE + z;

            int height = heightMap[x][z];

            if (height > SEA_LEVEL && GetTreeNoise(worldX, worldZ) > 0.98f)
                SpawnTree(worldX, height + 1, worldZ);
        }
    }

    chunk.SetTerrainGenerated(true);
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

    for (auto& [coord, chunk] : m_Chunks)
    {
        glm::vec3 camPosXZ = glm::vec3(camera.GetPosition().x, 0    , camera.GetPosition().z);
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

        chunk->Update();
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