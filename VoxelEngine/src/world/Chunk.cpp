#include "Chunk.h"
#include "Block.h"
#include "../VertexBufferLayout.h"
#include <iostream>
#include "../vendor/FastNoiseLite.h"

Chunk::Chunk(glm::ivec2 position) : m_ChunkPosition(position), m_VA(nullptr), m_VB(nullptr), m_IB(nullptr)
{
    for (int x = 0; x < WIDTH; x++)
        for (int y = 0; y < HEIGHT; y++)
            for (int z = 0; z < WIDTH; z++)
                m_Blocks[x][y][z] = Block(BlockType::AIR);
}

Chunk::~Chunk()
{
    delete m_VA;
    delete m_VB;
    delete m_IB;
}

void Chunk::GenerateMesh()
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int x = 0; x < WIDTH; x++)
        for (int y = 0; y < HEIGHT; y++)
            for (int z = 0; z < WIDTH; z++)
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

	float tileSize = 1.0f / 32.0f;
	BlockType blockType = GetBlockType(x, y, z);
	int tileX = 0 + blockType - 1;
	int tileY = 31; // (0, 0) in OpenGL is the bottom-left, so to get the first texture we need to set Y to 31


    for (int i = 0; i < verticesPerBlock; i++)
    {
        int v = i * floatsPerVertex;

        vertices.push_back(cubeVertices[v + 0] + x + (m_ChunkPosition.x * WIDTH));
        vertices.push_back(cubeVertices[v + 1] + y);                                
        vertices.push_back(cubeVertices[v + 2] + z + (m_ChunkPosition.y * WIDTH));
        
        float u = (tileX + cubeVertices[v + 3]) * tileSize;
        float v_coord = (tileY + cubeVertices[v + 4]) * tileSize;

        //vertices.push_back(cubeVertices[v + 3]); // U
        //vertices.push_back(cubeVertices[v + 4]); // V

        vertices.push_back(u);
        vertices.push_back(v_coord);

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

void Chunk::SetBlock(int x, int y, int z, BlockType type)
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && z >= 0 && z < WIDTH)
        m_Blocks[x][y][z] = Block(type);
}

void Chunk::SetSelectedBlock(bool hasBlock, glm::ivec3 position)
{
    m_HasSelectedBlock = hasBlock;
    m_SelectedBlock = position;
}