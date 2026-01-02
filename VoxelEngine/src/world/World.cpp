#include "World.h"

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

void World::GenerateChunk(int cx, int cz)
{
    Chunk& chunk = CreateChunk(cx, cz);

    constexpr int MAX_HEIGHT = 76;
    constexpr int MIN_HEIGHT = 50;

    for (int x = 0; x < 16; x++)
    {
        for (int z = 0; z < 16; z++)
        {
            int worldX = cx * 16 + x;
            int worldZ = cz * 16 + z;

            float noise = m_Noise.GetNoise((float)worldX, (float)worldZ);
            int height = MIN_HEIGHT + noise * (MAX_HEIGHT - MIN_HEIGHT);

            for (int y = 0; y < 128; y++)
            {
                if (y > height)
                    chunk.SetBlock(x, y, z, BlockType::AIR);
                else if (y < 5)
                    chunk.SetBlock(x, y, z, BlockType::STONE);
                else
                    chunk.SetBlock(x, y, z, BlockType::GRASS);
            }
        }
    }

    chunk.GenerateMesh();
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

    chunk.GenerateMesh();
}

void World::Render(Renderer& renderer, Shader& shader)
{
    for (auto& [coord, chunk] : m_Chunks)
    {
        chunk->Render(renderer, shader);
    }
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