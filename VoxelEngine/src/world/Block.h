#pragma once

enum BlockType
{
	AIR = 0,
	DIRT = 1,
	GRASS = 2,
	STONE = 3,
};

class Block
{
private:
	BlockType m_Type;
public:
	Block() : m_Type(BlockType::AIR) {}
	Block(BlockType type) : m_Type(type) {}
	BlockType GetType() const { return m_Type; }
};
