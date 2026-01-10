#pragma once

enum BlockType
{
	AIR = 0,
	GRASS = 1,
	STONE = 2,
	WOOD = 3,
	LEAF = 4,
	DIRT = 5,
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
