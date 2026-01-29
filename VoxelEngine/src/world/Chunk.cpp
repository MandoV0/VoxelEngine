#include "Chunk.h"
#include "Block.h"
#include "../VertexBufferLayout.h"
#include <iostream>
#include "../vendor/FastNoiseLite.h"

Chunk::Chunk(glm::ivec2 position) : m_ChunkPosition(position), m_VA(nullptr), m_VB(nullptr), m_IB(nullptr)
{
    
}

Chunk::~Chunk()
{
    delete m_VA;
    delete m_VB;
    delete m_IB;
}

BlockType Chunk::GetBlockTypeFromData(const ChunkData& data, int x, int y, int z)
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && z >= 0 && z < WIDTH)
        return data.blocks[x][y][z].GetType();
    return BlockType::AIR;
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

    

	renderTop = true;

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
            vertices.push_back({
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                });
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
            vertices.push_back({
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                });
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
            vertices.push_back({
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                });
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
            vertices.push_back({
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                });
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
            vertices.push_back({
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + -1 + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                });
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
            vertices.push_back({
                    vert.x + x + chunkPos.x * WIDTH,
                    vert.y + y,
                    vert.z + z + chunkPos.y * WIDTH,
                    (tileX + vert.u) * tileSize,
                    (tileY + vert.v) * tileSize,
                    vert.ao,
                    vert.lightLevel
                });
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

void Chunk::GenerateMeshWorker(Chunk* chunk, ChunkData data, glm::ivec2 position)
{
    std::vector<Vertex> localVertices;
    std::vector<unsigned int> localIndices;

    // Use a conservative reserve to avoid reallocations
    localVertices.reserve(WIDTH * HEIGHT * WIDTH * 6);
    localIndices.reserve(WIDTH * HEIGHT * WIDTH * 6);

    for (int x = 0; x < WIDTH; x++)
        for (int y = 0; y < HEIGHT; y++)
            for (int z = 0; z < WIDTH; z++)
            {
                if (GetBlockTypeFromData(data, x, y, z) != BlockType::AIR)
                    CreateBlockWorker(data, position, localVertices, localIndices, x, y, z);
            }

    // Pass data back to the chunk
    {
        std::lock_guard<std::mutex> lock(chunk->m_MeshMutex);
        chunk->m_IntermediateVertices = std::move(localVertices);
        chunk->m_IntermediateIndices = std::move(localIndices);
        chunk->m_HasNewMesh = true;
        chunk->m_IsGenerating = false;
    }
}

void Chunk::Update()
{
    // 1. If we have a new mesh ready from the thread, upload it
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

    // 2. If blocks changed and we aren't already working, start a thread
    if (m_IsDirty && !m_IsGenerating)
    {
        m_IsGenerating = true;
        m_IsDirty = false;

        // Create a copy of the data (Snapshot)
        ChunkData dataSnapshot = m_Blocks;

        // Launch the thread
        std::thread worker(GenerateMeshWorker, this, dataSnapshot, m_ChunkPosition);
        worker.detach();
    }
}

void Chunk::Render(Renderer& renderer, Shader& shader)
{
    if (!m_VA || !m_IB) return;
    if (m_IB->GetCount() == 0) return;
    renderer.Draw(*m_VA, *m_IB, shader);
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

bool Chunk::IsSolid(BlockType type)
{
	return type != BlockType::AIR && type != BlockType::LEAF; // Add all Transparent textures/blocks here so Solid faces adjacent to them are rendered
}