#include "Chunk.h"
#include "Block.h"
#include "../VertexBufferLayout.h"
#include <iostream>

Chunk::Chunk(glm::ivec3 position) : m_Position(position), m_VA(nullptr), m_VB(nullptr), m_IB(nullptr)
{
    for (int x = 0; x < 16; x++)
        for (int y = 0; y < 16; y++)
            for (int z = 0; z < 16; z++)
                m_Blocks[x][y][z] = Block(BlockType::AIR);
}

void Chunk::GenerateMesh()
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int x = 0; x < 16; x++)
        for (int y = 0; y < 16; y++)
            for (int z = 0; z < 16; z++)
            {
                if (GetBlockType(x, y, z) != BlockType::AIR)
                    CreateBlock(vertices, indices, x, y, z);
            }

    std::cout << "Chunk mesh: verts = " << vertices.size()
        << ", indices = " << indices.size() << std::endl;

    if (vertices.empty() || indices.empty())
    {
        delete m_VA;
        delete m_VB;
        delete m_IB;
        m_VA = nullptr;
        m_VB = nullptr;
        m_IB = nullptr;
        return;
    }

    delete m_VA;
    delete m_VB;
    delete m_IB;

    m_VA = new VertexArray();
    m_VA->Bind();

    m_VB = new VertexBuffer(vertices.data(), vertices.size() * sizeof(float));

    VertexBufferLayout layout;
	layout.Push<float>(3); // X, Y, Z
	layout.Push<float>(2); // U, V
	layout.Push<float>(1); // for per vertex Ambient Occlusion
    
    m_VA->AddBuffer(*m_VB, layout);

    m_IB = new IndexBuffer(indices.data(), indices.size());

    m_VA->Unbind();
}

void Chunk::Render(Renderer& renderer, Shader& shader)
{
    if (!m_VA || !m_IB) return;
    if (m_IB->GetCount() == 0) return;
    renderer.Draw(*m_VA, *m_IB, shader);
}

void Chunk::CreateBlock(std::vector<float>& vertices, std::vector<unsigned int>& indices, int x, int y, int z)
{
    static const float cubeVertices[] = {
        // Front
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // Bottom Left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // Bottom Right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // Top Right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Top Left

        // Back
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

         // Left
         -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

         // Right
          0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,

          // Top
          -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
           0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
           0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
          -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

          // Bottom
          -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
           0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
           0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
          -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f
    };

    static const unsigned int cubeIndices[] = {
         0,  1,  2,   2,  3,  0,   // Front
         4,  5,  6,   6,  7,  4,   // Back
         8,  9, 10,  10, 11,  8,   // Left
        12, 13, 14,  14, 15, 12,   // Right
        16, 17, 18,  18, 19, 16,   // Top
        20, 21, 22,  22, 23, 20    // Bottom
    };

    constexpr int floatsPerVertex = 6;
    constexpr int verticesPerBlock = 24;

    unsigned int baseIndex = vertices.size() / floatsPerVertex;

    for (int i = 0; i < verticesPerBlock; i++)
    {
        int v = i * floatsPerVertex;

        vertices.push_back(cubeVertices[v + 0] + x);
        vertices.push_back(cubeVertices[v + 1] + y);
        vertices.push_back(cubeVertices[v + 2] + z);

        vertices.push_back(cubeVertices[v + 3]); // u
        vertices.push_back(cubeVertices[v + 4]); // v

		vertices.push_back(cubeVertices[v + 5]);
    }

    for (unsigned int idx : cubeIndices)
    {
        indices.push_back(baseIndex + idx);
    }
}

BlockType Chunk::GetBlockType(int x, int y, int z)
{
    return m_Blocks[x][y][z].GetType();
}

bool Chunk::Raycast(glm::vec3 origin, glm::vec3 direction, float maxDistance, glm::ivec3& hitBlock, glm::ivec3& placeBlock)
{
    glm::vec3 chunkWorldOffset = glm::vec3(m_Position) * 16.0f;
    glm::vec3 start = origin - chunkWorldOffset; // Work in local chunk space
    
    // The DDA algorithm for Raycasting Voxel grids works best with a grid starting at 0. So we shift our coordinate space by 0.5.
    glm::vec3 pos = start + 0.5f;

    glm::ivec3 mapPos = glm::ivec3(glm::floor(pos));
    
    glm::vec3 rayDir = glm::normalize(direction);
    
    glm::vec3 deltaDist;
    deltaDist.x = std::abs(1.0f / rayDir.x);
    deltaDist.y = std::abs(1.0f / rayDir.y);
    deltaDist.z = std::abs(1.0f / rayDir.z);

    glm::ivec3 step;
    glm::vec3 sideDist;

    if (rayDir.x < 0)
    {
        step.x = -1;
        sideDist.x = (pos.x - mapPos.x) * deltaDist.x;
    }
    else
    {
        step.x = 1;
        sideDist.x = (mapPos.x + 1.0f - pos.x) * deltaDist.x;
    }

    if (rayDir.y < 0)
    {
        step.y = -1;
        sideDist.y = (pos.y - mapPos.y) * deltaDist.y;
    }
    else
    {
        step.y = 1;
        sideDist.y = (mapPos.y + 1.0f - pos.y) * deltaDist.y;
    }

    if (rayDir.z < 0)
    {
        step.z = -1;
        sideDist.z = (pos.z - mapPos.z) * deltaDist.z;
    }
    else
    {
        step.z = 1;
        sideDist.z = (mapPos.z + 1.0f - pos.z) * deltaDist.z;
    }

    // Perform DDA
    float dist = 0.0f;
    glm::ivec3 lastMapPos = mapPos;
    int maxSteps = static_cast<int>(maxDistance * 2.0f) + 10;

    for (int i = 0; i < maxSteps; i++)
    {
        if (mapPos.x >= 0 && mapPos.x < 16 &&
            mapPos.y >= 0 && mapPos.y < 16 &&
            mapPos.z >= 0 && mapPos.z < 16)
        {
            if (GetBlockType(mapPos.x, mapPos.y, mapPos.z) != BlockType::AIR)
            {
                hitBlock = mapPos;
                placeBlock = lastMapPos;
                return true;
            }
        }

        lastMapPos = mapPos;

        if (sideDist.x < sideDist.y)
        {
            if (sideDist.x < sideDist.z)
            {
                sideDist.x += deltaDist.x;
                mapPos.x += step.x;
                dist = sideDist.x;
            }
            else
            {
                sideDist.z += deltaDist.z;
                mapPos.z += step.z;
                dist = sideDist.z;
            }
        }
        else
        {
            if (sideDist.y < sideDist.z)
            {
                sideDist.y += deltaDist.y;
                mapPos.y += step.y;
                dist = sideDist.y;
            }
            else
            {
                sideDist.z += deltaDist.z;
                mapPos.z += step.z;
                dist = sideDist.z;
            }
        }

        glm::vec3 currWorldPos = (glm::vec3(mapPos) - 0.5f);
        if (glm::distance(start, currWorldPos) > maxDistance) break;
    }

    return false;
}

void Chunk::SetBlock(int x, int y, int z, BlockType type)
{
    if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16)
        m_Blocks[x][y][z] = Block(type);
}

void Chunk::SetSelectedBlock(bool hasBlock, glm::ivec3 position)
{
    m_HasSelectedBlock = hasBlock;
    m_SelectedBlock = position;
}

void Chunk::RenderBlockOutline(Renderer& renderer, Shader& shader)
{
    if (!m_HasSelectedBlock) return;

    glLineWidth(2.0f);

    float x = m_SelectedBlock.x;
    float y = m_SelectedBlock.y;
    float z = m_SelectedBlock.z;
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