#include "World.h"

#include "../VertexBufferLayout.h"

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

    //chunk.GenerateMesh();
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

    //chunk.GenerateMesh();
}

void World::Render(Renderer& renderer, Shader& shader)
{
    for (auto& [coord, chunk] : m_Chunks)
    {
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

    while (distanceTraveled <= maxDistance)
    {
		Chunk* chunk = GetChunk(WorldToChunk(x), WorldToChunk(z)); // To traverse mutliple chunks, at each step we check in which Chunk the World Coordinates are.
        if (chunk)
        {
            BlockType type = chunk->GetBlockType(WorldToLocal(x), y, WorldToLocal(z));

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

int World::WorldToChunk(int x)
{
    return floor((float)x / 16);
}

int World::WorldToLocal(int x)
{
    int r = x % 16;
    return r < 0 ? r + 16 : r;
}