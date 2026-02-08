#include "Chunk.h"
#include "Block.h"
#include "../VertexBufferLayout.h"
#include <iostream>
#include "../vendor/FastNoiseLite.h"

#include "World.h"


Chunk::Chunk(glm::ivec2 position) : m_ChunkPosition(position), m_VA(nullptr), m_VB(nullptr), m_IB(nullptr),
m_WaterVA(nullptr), m_WaterVB(nullptr), m_WaterIB(nullptr)
{
    
}

Chunk::~Chunk()
{
    delete m_VA;
    delete m_VB;
    delete m_IB;

    delete m_WaterVA;
    delete m_WaterVB;
    delete m_WaterIB;
}

BlockType Chunk::GetBlockTypeFromData(const ChunkData& data, int x, int y, int z)
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && z >= 0 && z < WIDTH)
        return data.blocks[x][y][z].GetType();
    return BlockType::AIR;
}

BlockType Chunk::GetBlockTypeFromData(const PaddedChunkData& data, int x, int y, int z)
{
	// x, y, z can bow be -1 or WIDTH (for neighbor checks)
    // So we map this tothe padded array: [0 to WIDTH+1]
    int px = x + 1;  // Offset
    int py = y;
    int pz = z + 1;  // Offset

    if (px >= 0 && px < WIDTH + 2 && py >= 0 && py < HEIGHT && pz >= 0 && pz < WIDTH + 2)
        return data.blocks[px][py][pz].GetType();
    return BlockType::AIR;;
}

// TODO, check across chunk boundaries
void Chunk::CreateBlockWorker(const ChunkData& data, glm::ivec2 chunkPos, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int x, int y, int z)
{
    // Air is not a real block so we skip it
    BlockType blockType = GetBlockTypeFromData(data, x, y, z);
    if (blockType == BlockType::AIR) return;

    bool renderFront = !IsSolid(GetBlockTypeFromData(data, x, y, z + 1));
    bool renderBack = !IsSolid(GetBlockTypeFromData(data, x, y, z - 1));
    bool renderLeft = !IsSolid(GetBlockTypeFromData(data, x - 1, y, z));
    bool renderRight = !IsSolid(GetBlockTypeFromData(data, x + 1, y, z));
    bool renderTop = !IsSolid(GetBlockTypeFromData(data, x, y + 1, z));
    bool renderBottom = !IsSolid(GetBlockTypeFromData(data, x, y - 1, z));

    // Skip if all faces are hdden
    if (!renderFront && !renderBack && !renderLeft &&
        !renderRight && !renderTop && !renderBottom) {
        return;
    }

    float tileSize = 1.0f / 32.0f;
    int tileX = 0 + blockType - 1;
    int tileY = 31;

	// float lightLevel = GetLightLevelAt(x, y, z, data);
    float lightLevel = 1;

    // Face data: positions, UV, AO, LightLevel (experimental)
    struct FaceVertex {
        float x, y, z, u, v, ao, lightLevel;
    };

    // PRECOMPUTE
    float wx = x + chunkPos.x * WIDTH;
    float wy = y;
    float wz = z + chunkPos.y * WIDTH;

    // Front face (+Z)
    if (renderFront) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex frontFace[] = {
            {-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, lightLevel},
            { 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, lightLevel},
            { 0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {-0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : frontFace) {
            vertices.emplace_back(
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Back face (-Z)
    if (renderBack) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex backFace[] = {
            { 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, lightLevel},
            {-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, lightLevel},
            {-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            { 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : backFace) {
            vertices.emplace_back(
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Left face (-X)
    if (renderLeft) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex leftFace[] = {
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, lightLevel},
            {-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, lightLevel},
            {-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : leftFace) {
            vertices.emplace_back(
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Right face (+X)
    if (renderRight) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex rightFace[] = {
            {0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, lightLevel},
            {0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, lightLevel},
            {0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : rightFace) {
            vertices.emplace_back(
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Top face (+Y)
    if (renderTop) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex topFace[] = {
            {-0.5f, 0.5f,  0.5f, 0.0f, 0.0f, 0.0f, lightLevel},
            { 0.5f, 0.5f,  0.5f, 1.0f, 0.0f, 0.0f, lightLevel},
            { 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : topFace) {
            vertices.emplace_back(
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + -1 + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Bottom face (-Y)
    if (renderBottom) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex bottomFace[] = {
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, lightLevel},
            { 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, lightLevel},
            { 0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : bottomFace) {
            vertices.emplace_back(
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

	// TODO: Only apply AO when their is a block below or below + front direction, Check for front, back left, right. But how would this work across chunk boundaries and performance wise.
}

void Chunk::CreateBlockWorker(const PaddedChunkData& data, glm::ivec2 chunkPos, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int x, int y, int z)
{
    // Air is not a real block so we skip it
    BlockType blockType = GetBlockTypeFromData(data, x, y, z);
    if (blockType == BlockType::AIR) return;

    bool renderFront = !IsSolid(GetBlockTypeFromData(data, x, y, z + 1));
    bool renderBack = !IsSolid(GetBlockTypeFromData(data, x, y, z - 1));
    bool renderLeft = !IsSolid(GetBlockTypeFromData(data, x - 1, y, z));
    bool renderRight = !IsSolid(GetBlockTypeFromData(data, x + 1, y, z));
    bool renderTop = !IsSolid(GetBlockTypeFromData(data, x, y + 1, z));
    bool renderBottom = !IsSolid(GetBlockTypeFromData(data, x, y - 1, z));

	if (blockType == BlockType::WATER) {
        // For water, we only render the top face if the block above is not water, and we render the sides if the neighboring block is not solid (to create a "flow" effect)
        renderTop = !IsSolid(GetBlockTypeFromData(data, x, y + 1, z)) && GetBlockTypeFromData(data, x, y + 1, z) != BlockType::WATER;
        renderFront = !IsSolid(GetBlockTypeFromData(data, x, y, z + 1)) && GetBlockTypeFromData(data, x, y, z + 1) != BlockType::WATER;
        renderBack = !IsSolid(GetBlockTypeFromData(data, x, y, z - 1)) && GetBlockTypeFromData(data, x, y, z - 1) != BlockType::WATER;
        renderLeft = !IsSolid(GetBlockTypeFromData(data, x - 1, y, z)) && GetBlockTypeFromData(data, x - 1, y, z) != BlockType::WATER;
        renderRight = !IsSolid(GetBlockTypeFromData(data, x + 1, y, z)) && GetBlockTypeFromData(data, x + 1, y, z) != BlockType::WATER;
    }

    // Skip if all faces are hdden
    if (!renderFront && !renderBack && !renderLeft &&
        !renderRight && !renderTop && !renderBottom) {
        return;
    }

    float tileSize = 1.0f / 32.0f;
    int tileX = 0 + blockType - 1;
    int tileY = 31;

    // float lightLevel = GetLightLevelAt(x, y, z, data);
    float lightLevel = 1;

    // Face data: positions, UV, AO, LightLevel (experimental)
    struct FaceVertex {
        float x, y, z, u, v, ao, lightLevel;
    };

    // PRECOMPUTE
    float wx = x + chunkPos.x * WIDTH;
    float wy = y;
    float wz = z + chunkPos.y * WIDTH;

    // Front face (+Z)
    if (renderFront) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex frontFace[] = {
            {-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, lightLevel},
            { 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, lightLevel},
            { 0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {-0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : frontFace) {
            vertices.emplace_back(
                vert.x + x + chunkPos.x * WIDTH,
                vert.y + y,
                vert.z + z + chunkPos.y * WIDTH,
                (tileX + vert.u) * tileSize,
                (tileY + vert.v) * tileSize,
                vert.ao,
                vert.lightLevel
            );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Back face (-Z)
    if (renderBack) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex backFace[] = {
            { 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, lightLevel},
            {-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, lightLevel},
            {-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            { 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : backFace) {
            vertices.emplace_back(
                vert.x + x + chunkPos.x * WIDTH,
                vert.y + y,
                vert.z + z + chunkPos.y * WIDTH,
                (tileX + vert.u) * tileSize,
                (tileY + vert.v) * tileSize,
                vert.ao,
                vert.lightLevel
            );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Left face (-X)
    if (renderLeft) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex leftFace[] = {
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, lightLevel},
            {-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, lightLevel},
            {-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : leftFace) {
            vertices.emplace_back(
                vert.x + x + chunkPos.x * WIDTH,
                vert.y + y,
                vert.z + z + chunkPos.y * WIDTH,
                (tileX + vert.u) * tileSize,
                (tileY + vert.v) * tileSize,
                vert.ao,
                vert.lightLevel
            );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Right face (+X)
    if (renderRight) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex rightFace[] = {
            {0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, lightLevel},
            {0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, lightLevel},
            {0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : rightFace) {
            vertices.emplace_back(
                vert.x + x + chunkPos.x * WIDTH,
                vert.y + y,
                vert.z + z + chunkPos.y * WIDTH,
                (tileX + vert.u) * tileSize,
                (tileY + vert.v) * tileSize,
                vert.ao,
                vert.lightLevel
            );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Top face (+Y)
    if (renderTop) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex topFace[] = {
            {-0.5f, 0.5f,  0.5f, 0.0f, 0.0f, 0.0f, lightLevel},
            { 0.5f, 0.5f,  0.5f, 1.0f, 0.0f, 0.0f, lightLevel},
            { 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : topFace) {
            vertices.emplace_back(
                vert.x + x + chunkPos.x * WIDTH,
                vert.y + y,
                vert.z + z + chunkPos.y * WIDTH,
                (tileX + vert.u) * tileSize,
                (tileY + -1 + vert.v) * tileSize,
                vert.ao,
                vert.lightLevel
            );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // Bottom face (-Y)
    if (renderBottom) {
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size());
        FaceVertex bottomFace[] = {
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, lightLevel},
            { 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, lightLevel},
            { 0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, lightLevel},
            {-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, lightLevel}
        };

        for (const auto& vert : bottomFace) {
            vertices.emplace_back(
                vert.x + x + chunkPos.x * WIDTH,
                vert.y + y,
                vert.z + z + chunkPos.y * WIDTH,
                (tileX + vert.u) * tileSize,
                (tileY + vert.v) * tileSize,
                vert.ao,
                vert.lightLevel
            );
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    // TODO: Only apply AO when their is a block below or below + front direction, Check for front, back left, right. But how would this work across chunk boundaries and performance wise.
}

void Chunk::GenerateMeshWorker(Chunk* chunk, const PaddedChunkData data, glm::ivec2 position)
{
    std::vector<Vertex> localVertices;
    std::vector<unsigned int> localIndices;
    
    std::vector<Vertex> localWaterVertices;
    std::vector<unsigned int> localWaterIndices;

    localWaterVertices.reserve(2048);
    localWaterIndices.reserve(3072);

    // Use a conservative reserve to avoid reallocations
    localVertices.reserve(8192);
    localIndices.reserve(12288);

    for (int x = 0; x < WIDTH; x++)
        for (int y = 0; y < HEIGHT; y++)
            for (int z = 0; z < WIDTH; z++)
            {
				BlockType type = GetBlockTypeFromData(data, x, y, z);
                if (type == BlockType::AIR) continue;

                if (type == BlockType::WATER)
                {
					CreateBlockWorker(data, position, localWaterVertices, localWaterIndices, x, y, z);
                }
                else
                {
                    CreateBlockWorker(data, position, localVertices, localIndices, x, y, z);
                }
            }

    // Pass data back to the chunk
    {
        std::lock_guard<std::mutex> lock(chunk->m_MeshMutex);
        chunk->m_IntermediateVertices = std::move(localVertices);
        chunk->m_IntermediateIndices = std::move(localIndices);

        chunk->m_IntermediateWaterVertices = std::move(localWaterVertices);
        chunk->m_IntermediateWaterIndices = std::move(localWaterIndices);
        chunk->m_HasNewWaterMesh = true;

        chunk->m_HasNewMesh = true;
        chunk->m_IsGenerating = false;
    }
}

void Chunk::Update(World* world)
{
    // If we have a new mesh ready from the thread, upload it
    if (m_HasNewMesh)
    {
        std::lock_guard<std::mutex> lock(m_MeshMutex);

        delete m_VA; delete m_VB; delete m_IB;
        m_VA = nullptr; m_VB = nullptr; m_IB = nullptr;

        if (!m_IntermediateVertices.empty() && !m_IntermediateIndices.empty())
        {
            m_VA = new VertexArray();
            m_VA->Bind();

            m_VB = new VertexBuffer(m_IntermediateVertices.data(), m_IntermediateVertices.size() * sizeof(Vertex));

            VertexBufferLayout layout;
            layout.Push<float>(3); // X, Y, Z
            layout.Push<float>(2); // U, V
            layout.Push<float>(1); // Ambient Occlusion
			layout.Push<float>(1); // Light Level (experimental)

            m_VA->AddBuffer(*m_VB, layout);
            m_IB = new IndexBuffer(m_IntermediateIndices.data(), m_IntermediateIndices.size());

            m_VA->Unbind();
        }

        m_IntermediateVertices.clear();
        m_IntermediateIndices.clear();
        m_HasNewMesh = false;
    }

    if (m_HasNewWaterMesh)
    {
        std::lock_guard<std::mutex> lock(m_MeshMutex);

        // Upload water mesh
        delete m_WaterVA; delete m_WaterVB; delete m_WaterIB;
        m_WaterVA = nullptr; m_WaterVB = nullptr; m_WaterIB = nullptr;

        if (!m_IntermediateWaterVertices.empty()) {
            m_WaterVA = new VertexArray();
            m_WaterVA->Bind();
            m_WaterVB = new VertexBuffer(m_IntermediateWaterVertices.data(),
                m_IntermediateWaterVertices.size() * sizeof(Vertex));
            VertexBufferLayout layout;
            layout.Push<float>(3);
            layout.Push<float>(2);
            layout.Push<float>(1);
            layout.Push<float>(1);
            m_WaterVA->AddBuffer(*m_WaterVB, layout);
            m_WaterIB = new IndexBuffer(m_IntermediateWaterIndices.data(),
                m_IntermediateWaterIndices.size());
            m_WaterVA->Unbind();
        }

        m_IntermediateWaterVertices.clear();
        m_IntermediateWaterIndices.clear();
        m_HasNewWaterMesh = false;
    }

    if (m_IsDirty && !m_IsGenerating)
    {
        m_IsGenerating = true;
        m_IsDirty = false;

        glm::ivec2 pos = m_ChunkPosition;
        
        // Prepare Padded Data on Main Thread to avoid race conditions with SetBlock
        // Access neighbors safely on Main Thread
        Chunk* leftN = world->GetChunk(pos.x - 1, pos.y);
        Chunk* rightN = world->GetChunk(pos.x + 1, pos.y);
        Chunk* backN = world->GetChunk(pos.x, pos.y - 1);
        Chunk* frontN = world->GetChunk(pos.x, pos.y + 1);

        // Copy data
        // We create a shared_ptr to the padded data so we can move it into the lambda
        // Using shared_ptr avoids large stack copies if the lambda captures by value
        auto paddedData = std::make_shared<PaddedChunkData>();

        // Fill Center
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                for (int z = 0; z < WIDTH; z++) {
                    paddedData->blocks[x + 1][y][z + 1] = m_Blocks.blocks[x][y][z];
                }
            }
        }

        // Neigbor data
        if (leftN && leftN->IsTerrainGenerated()) {
            for (int y = 0; y < HEIGHT; y++)
                for (int z = 0; z < WIDTH; z++)
                    paddedData->blocks[0][y][z + 1] = leftN->m_Blocks.blocks[WIDTH - 1][y][z];
        }

        if (rightN && rightN->IsTerrainGenerated()) {
            for (int y = 0; y < HEIGHT; y++)
                for (int z = 0; z < WIDTH; z++)
                    paddedData->blocks[WIDTH + 1][y][z + 1] = rightN->m_Blocks.blocks[0][y][z];
        }

        if (backN && backN->IsTerrainGenerated()) {
            for (int x = 0; x < WIDTH; x++)
                for (int y = 0; y < HEIGHT; y++)
                    paddedData->blocks[x + 1][y][0] = backN->m_Blocks.blocks[x][y][WIDTH - 1];
        }

        if (frontN && frontN->IsTerrainGenerated()) {
            for (int x = 0; x < WIDTH; x++)
                for (int y = 0; y < HEIGHT; y++)
                    paddedData->blocks[x + 1][y][WIDTH + 1] = frontN->m_Blocks.blocks[x][y][0];
        }

        world->EnqueueJob([this, paddedData, pos]() {
            // Generate mesh using the snapshot
            GenerateMeshWorker(this, *paddedData, pos);
        });
    }
}


void Chunk::Render(Renderer& renderer, Shader& shader, int layer)
{
	if (layer == 0) {
        if (!m_VA || !m_IB) return;
        if (m_IB->GetCount() == 0) return;
        renderer.Draw(*m_VA, *m_IB, shader);
    } else if (layer == 2) {
        if (!m_WaterVA || !m_WaterIB) return;
        if (m_WaterIB->GetCount() == 0) return;
        renderer.Draw(*m_WaterVA, *m_WaterIB, shader);
    }
}

BlockType Chunk::GetBlockType(int x, int y, int z)
{
    return m_Blocks.blocks[x][y][z].GetType();
}

void Chunk::SetBlock(int x, int y, int z, BlockType type)
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && z >= 0 && z < WIDTH)
    {
        m_Blocks.blocks[x][y][z] = Block(type);
        m_IsDirty = true;
    }
}

void Chunk::SetSelectedBlock(bool hasBlock, glm::ivec3 position)
{
    m_HasSelectedBlock = hasBlock;
    m_SelectedBlock = position;
}

float Chunk::GetLightLevelAt(int x, int y, int z, const ChunkData& data)
{
    for (int checkY = y + 1; checkY < HEIGHT; checkY++)
    {
        if (IsSolid(GetBlockTypeFromData(data, x, checkY, z)))
            return 1.0f; // In shadow
    }
    return 1.0f; // Exposed to sky
}

bool Chunk::IsAir(int x, int y, int z)
{
    return GetBlockType(x, y, z) == BlockType::AIR;
}

// Add all Transparent textures/blocks here so Solid faces adjacent to them are rendered
bool Chunk::IsSolid(BlockType type)
{
	return  type != BlockType::AIR &&
            type != BlockType::LEAF &&
            type != BlockType::WATER; 
}