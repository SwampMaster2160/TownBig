#pragma once

union Version
{
	uint64_t full;
	uint16_t sub[4];
	struct
	{
		uint16_t snapshot;
		uint16_t minor;
		uint16_t mid;
		uint16_t major;
	};

	Version(uint16_t snapshot, uint16_t minor, uint16_t mid, uint16_t major)
	{
		this->snapshot = snapshot;
		this->minor = minor;
		this->mid = mid;
		this->major = major;
	}
};

union TriPoint
{
	GLdouble doubles[8];
	struct
	{
		GLdouble x;
		GLdouble y;
		GLdouble z;
		GLdouble blank0;
		GLdouble texX;
		GLdouble texY;
	};

	TriPoint(GLdouble x, GLdouble y, GLdouble z, GLdouble texX, GLdouble texY)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->texX = texX;
		this->texY = texY;
	}

	TriPoint()
	{

	}
};

struct PosSize
{
	size_t pos;
	size_t size;

	PosSize()
	{

	}

	PosSize(size_t pos, size_t size)
	{
		this->pos = pos;
		this->size = size;
	}

	size_t getEnd()
	{
		return pos + size;
	}
};

struct TextureData
{
	const char* name;
	double xStart;
	double yStart;
	double xEnd;
	double yEnd;
};

struct MainData
{
	bool redrawMap;
	sf::Vector2u windowedWindowSize;
	bool fullScreen;
	std::vector<TriPoint> triangles;
	std::vector<PosSize> freeTriangles;
	std::vector<TextureData> textureDatas;
};

enum class Inputs : uint8_t
{
	fullScreen, mainClick, pan, windowResize, zoomIn, zoomOut, size
};

enum class GroundMaterial : uint8_t
{
	grass, sand, size
};

struct Tile
{
	PosSize trisPosSize;

	GroundMaterial groundMaterial;
	int8_t height;
};

struct TileRow
{
	Tile tiles[256];

	TileRow()
	{

	}

	Tile &operator[] (size_t x)
	{
		return tiles[x];
	}
};

struct Map
{
	TileRow tiles[256];

	Map()
	{

	}

	TileRow& operator[] (size_t x)
	{
		return tiles[x];
	}
};

enum class TextureID : uint8_t
{
	grass, water, sand, rock, snow, select, debug, size
};