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

enum class GUIEnum : uint8_t
{
	ingame, pause, title, size
};

enum class GUIElementEnum : uint8_t
{
	rect
};

struct GUIElement
{
	GUIElementEnum type;
	void* element;

	GUIElement()
	{

	}

	GUIElement(GUIElementEnum type, void* element)
	{
		this->type = type;
		this->element = element;
	}
};

struct GUI
{
	std::vector<GUIElement> elements;

	GUI()
	{
		elements = {};
	}
};

enum class ScreenPosHAlign : uint8_t
{
	l, c, r
};

enum class ScreenPosVAlign : uint8_t
{
	t, c, b
};

struct ScreenPos
{
	sf::Vector2f pos;
	ScreenPosHAlign hAlign;
	ScreenPosVAlign vAlign;

	ScreenPos()
	{

	}

	ScreenPos(sf::Vector2f pos, ScreenPosHAlign hAlign, ScreenPosVAlign vAlign)
	{
		this->pos = pos;
		this->hAlign = hAlign;
		this->vAlign = vAlign;
	}
};

struct GUIRect
{
	ScreenPos pos;
	ScreenPos end;
	sf::Color color;

	GUIRect()
	{

	}

	GUIRect(ScreenPos pos, ScreenPos end, sf::Color color)
	{
		this->pos = pos;
		this->end = end;
		this->color = color;
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

enum class TextureID : uint8_t
{
	grass, water, sand, rock, snow, select, debug, waterSide, pineTree, rockPile, oakTree, size
};

enum class GroundMaterialEnum : uint8_t
{
	grass, sand, rock, snow, size
};

struct GroundMaterialData
{
	TextureID texture;
};

enum class FoliageEnum : uint8_t
{
	pineTree, rockPile, oakTree, size
};

struct FoliageData
{
	TextureID texture;
};

enum class LandOccEnum : uint8_t
{
	none = 0, foliage, debug, size
};

struct LandOcc
{
	LandOccEnum type;
	char data;
};

enum class Inputs : uint8_t
{
	fullScreen, mainClick, pan, windowResize, zoomIn, zoomOut, f1, f2, f3, f4, size
};

union Tile
{
	struct
	{
		PosSize trisPosSize;

		GroundMaterialEnum groundMaterial;
		int8_t height;
		LandOcc landOcc;
	};
	uint8_t data[64];

	Tile()
	{

	}
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

enum class MapTerrainType : uint8_t
{
	regular, flatGrass, flatWater, swamp
};

struct MainData
{
	bool redrawMap;
	sf::Vector2u windowedWindowSize;
	sf::Vector2u windowSize;
	uint16_t guiScale;
	bool fullScreen;
	std::vector<TriPoint> triangles;
	std::vector<PosSize> freeTriangles;
	std::vector<TextureData> textureDatas;
	GroundMaterialData groundMaterialDatas[(uint8_t)GroundMaterialEnum::size];
	FoliageData foliageDatas[(uint8_t)FoliageEnum::size];
	GUI guis[(size_t)GUIEnum::size];
	GUIEnum currentGUI;
};