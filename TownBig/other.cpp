#include "main.hpp"
#include <iostream>
#include <chrono>

void initWindow(sf::RenderWindow& window, MainData& windowData, bool firstTime)
{
	sf::VideoMode mode = sf::VideoMode::getDesktopMode();
	sf::Vector2u screenSize = { mode.width, mode.height };
	if (firstTime) windowData.windowedWindowSize = { screenSize.x / 2, screenSize.y / 2 };

	sf::ContextSettings contextSettings;
	contextSettings.depthBits = 32;
	contextSettings.sRgbCapable = 0;
	contextSettings.antialiasingLevel = 1;

	if (windowData.fullScreen)
	{
		window.create({ screenSize.x, screenSize.y }, "TownBig", sf::Style::Fullscreen, contextSettings);
	}
	else
	{
		window.create({ windowData.windowedWindowSize.x, windowData.windowedWindowSize.y }, "TownBig", sf::Style::Default, contextSettings);
	}

	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearDepth(1.f);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glViewport(0, 0, window.getSize().x, window.getSize().y);
	glMatrixMode(GL_PROJECTION);
	GLdouble ratio = (double)(window.getSize().x) / window.getSize().y;
	glOrtho(-ratio, ratio, -1., 1., 1., 1000.);

	//window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(1);
	window.setActive(1);
}

PosSize appendTris(MainData& mainData, std::vector<TriPoint>& newTris)
{
	for (size_t x = 0; x < mainData.freeTriangles.size(); x++)
	{
		if (mainData.freeTriangles[x].size >= newTris.size())
		{
			PosSize out = { mainData.freeTriangles[x].pos, newTris.size() };
			memcpy(&mainData.triangles[mainData.freeTriangles[x].pos], &newTris[0], newTris.size() * sizeof(TriPoint));
			mainData.freeTriangles[x].pos += newTris.size();
			mainData.freeTriangles[x].size -= newTris.size();

			if (mainData.freeTriangles[x].size == 0)
			{
				mainData.freeTriangles.erase(mainData.freeTriangles.begin() + x);
			}

			return out;
		}
	}

	size_t triVectorSize = mainData.triangles.size();
	mainData.triangles.resize(triVectorSize + newTris.size());
	memcpy(&mainData.triangles[triVectorSize], &newTris[0], newTris.size() * sizeof(TriPoint));
	return { triVectorSize, newTris.size() };
}

void deleteTris(MainData& mainData, PosSize posSize)
{
	for (size_t x = posSize.pos; x < posSize.getEnd(); x++)
	{
		for (size_t y = 0; y < 3; y++)
		{
			mainData.triangles[x].doubles[y] = 0;
		}
	}
	mainData.freeTriangles.push_back(posSize);

	for (size_t x = 0; x < mainData.freeTriangles.size(); x++)
	{
		if (posSize.pos == mainData.freeTriangles[x].getEnd())
		{
			mainData.freeTriangles[mainData.freeTriangles.size() - 1].pos -= mainData.freeTriangles[x].size;
			mainData.freeTriangles[mainData.freeTriangles.size() - 1].size += mainData.freeTriangles[x].size;
			mainData.freeTriangles.erase(mainData.freeTriangles.begin() + x);
		}
	}

	for (size_t x = 0; x < mainData.freeTriangles.size(); x++)
	{
		if (posSize.getEnd() == mainData.freeTriangles[x].pos)
		{
			mainData.freeTriangles[mainData.freeTriangles.size() - 1].size += mainData.freeTriangles[x].size;
			mainData.freeTriangles.erase(mainData.freeTriangles.begin() + x);
		}
	}
}

void createQuad(sf::Vector3<double> pos0, sf::Vector3<double> pos1, sf::Vector3<double> pos2, sf::Vector3<double> pos3, std::vector<TriPoint>& newTris, TextureData& textureData)
{
	newTris.push_back({ pos0.x, pos0.y, pos0.z, textureData.xStart, textureData.yStart });
	newTris.push_back({ pos1.x, pos1.y, pos1.z, textureData.xEnd,   textureData.yStart });
	newTris.push_back({ pos3.x, pos3.y, pos3.z, textureData.xEnd,   textureData.yEnd });
	newTris.push_back({ pos0.x, pos0.y, pos0.z, textureData.xStart, textureData.yStart });
	newTris.push_back({ pos3.x, pos3.y, pos3.z, textureData.xEnd,   textureData.yEnd });
	newTris.push_back({ pos2.x, pos2.y, pos2.z, textureData.xStart, textureData.yEnd });
}

bool isTileInMap(sf::Vector2<int16_t> pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.x > 255 || pos.y > 255) return 0;
	return 1;
}

uint8_t random(uint64_t seed, uint64_t counter)
{
	uint64_t a = (seed * counter * counter);
	uint64_t b = (seed * seed * counter);
	uint8_t c = (a * b) % 7;
	return (a + b + c);
}

uint8_t random(uint64_t seed, uint64_t x, uint64_t y, uint64_t z)
{
	return random(seed, x + y * 0xFFFFEEE + z * 0x100000000000000);
}

uint64_t random64(uint64_t seed, uint64_t counter)
{
	return (uint64_t)random(seed, counter) + (uint64_t)random(seed, counter + 1) * 0x100 + (uint64_t)random(seed, counter + 2) * 0x10000 + (uint64_t)random(seed, counter + 3) * 0x1000000 + (uint64_t)random(seed, counter + 4) * 0x100000000 + (uint64_t)random(seed, counter + 5) * 0x10000000000 + (uint64_t)random(seed, counter + 6) * 0x1000000000000 + (uint64_t)random(seed, counter + 7) * 0x100000000000000;
}

double filter(sf::Vector2<double> pos, sf::Vector2<double> cPos, double size, double h0, double h1, double h2, double h3)
{
	double g0 = (h1 - h0) / size;
	double p0 = g0 * pos.x + (h0 - g0 * cPos.x);
	double g1 = (h3 - h2) / size;
	double p1 = g1 * pos.x + (h2 - g1 * cPos.x);

	double g2 = (p1 - p0) / size;
	double p2 = g2 * pos.y + (p0 - g2 * cPos.y);

	return p2;
}

void drawTile(MainData& mainData, Tile& tile, sf::Vector2<uint8_t> pos, Map& map)
{
	if (!mainData.redrawMap)
	{
		deleteTris(mainData, tile.trisPosSize);
	}

	// Get neighbors height

	int8_t neighborHeights[3][3];

	for (uint8_t x = 0; x < 3; x++)
	{
		int16_t xMap = (int16_t)pos.x - (int16_t)1 + (int16_t)x;
		for (uint8_t y = 0; y < 3; y++)
		{
			int16_t yMap = (int16_t)pos.y - (int16_t)1 + (int16_t)y;

			if (isTileInMap({ xMap, yMap }))
			{
				neighborHeights[x][y] = map[xMap][yMap].height;
			}
			else neighborHeights[x][y] = -128;
		}
	}

	GroundMaterialEnum groundMaterial = tile.groundMaterial;
	int8_t height = tile.height;
	TextureData& textureData = mainData.textureDatas[(size_t)mainData.groundMaterialDatas[(size_t)groundMaterial].texture];
	std::vector<TriPoint> newTris = {};

	// Terrain

	createQuad(
		{ (double)pos.x, (double)height, (double)pos.y },
		{ (double)pos.x + 1, (double)height, (double)pos.y },
		{ (double)pos.x, (double)height, (double)pos.y + 1 },
		{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
		newTris, textureData);

	// Slopes

	bool drawSlope = tile.landOcc.type == LandOccEnum::none;

	if (drawSlope)
	{
		if (neighborHeights[1][0] == height + 1)
		{
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				newTris, textureData);
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData);
		}
		if (neighborHeights[2][1] == height + 1)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData);
			createQuad(
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 0.9999 },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 0.9999 },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 0.9999 },
				{ (double)pos.x, (double)height, (double)pos.y + 0.9999 },
				newTris, textureData);
		}
		if (neighborHeights[1][2] == height + 1)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y },
				newTris, textureData);
			createQuad(
				{ (double)pos.x + 0.0001, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x + 0.0001, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x + 0.0001, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 0.0001, (double)height, (double)pos.y },
				newTris, textureData);
		}
		if (neighborHeights[0][1] == height + 1)
		{
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				newTris, textureData);
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				newTris, textureData);
		}

		if (neighborHeights[0][0] == height + 1 && neighborHeights[0][1] == height && neighborHeights[1][0] == height)
		{
			createQuad(
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				newTris, textureData);
		}
		if (neighborHeights[2][0] == height + 1 && neighborHeights[1][0] == height && neighborHeights[2][1] == height)
		{
			createQuad(
				{ (double)pos.x, (double)height, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				newTris, textureData);
		}
		if (neighborHeights[2][2] == height + 1 && neighborHeights[2][1] == height && neighborHeights[1][2] == height)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData);
		}
		if (neighborHeights[2][2] == height + 1 && neighborHeights[2][1] == height && neighborHeights[1][2] == height)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData);
		}
	}

	// Clifs

	for (int8_t x = neighborHeights[0][1]; x <= height; x++)
	{
		createQuad(
			{ (double)pos.x, (double)x, (double)pos.y },
			{ (double)pos.x, (double)x, (double)pos.y + 1 },
			{ (double)pos.x, (double)x - 1, (double)pos.y },
			{ (double)pos.x, (double)x - 1, (double)pos.y + 1 },
			newTris, textureData);
	}
	for (int8_t x = neighborHeights[1][2]; x <= height; x++)
	{
		createQuad(
			{ (double)pos.x, (double)x, (double)pos.y + 1 },
			{ (double)pos.x + 1, (double)x, (double)pos.y + 1 },
			{ (double)pos.x, (double)x - 1, (double)pos.y + 1 },
			{ (double)pos.x + 1, (double)x - 1, (double)pos.y + 1 },
			newTris, textureData);
	}

	// Water

	if (height < 0)
	{
		createQuad(
			{ (double)pos.x, -0.0625, (double)pos.y },
			{ (double)pos.x + 1, -0.0625, (double)pos.y },
			{ (double)pos.x, -0.0625, (double)pos.y + 1 },
			{ (double)pos.x + 1, -0.0625, (double)pos.y + 1 },
			newTris, mainData.textureDatas[(size_t)TextureID::water]);
	}

	if (pos.x == 0)
	{
		for (int8_t x = height + 1; x < 1; x++)
		{
			if (x == 0)
			{
				createQuad(
					{ (double)0.002, (double)x, (double)pos.y },
					{ (double)0.002, (double)x, (double)pos.y + 1 },
					{ (double)0.002, (double)x - 1, (double)pos.y },
					{ (double)0.002, (double)x - 1, (double)pos.y + 1 },
					newTris, mainData.textureDatas[(size_t)TextureID::waterSide]);
			}
			else
			{
				createQuad(
					{ (double)0.0002, (double)x, (double)pos.y },
					{ (double)0.0002, (double)x, (double)pos.y + 1 },
					{ (double)0.0002, (double)x - 1, (double)pos.y },
					{ (double)0.0002, (double)x - 1, (double)pos.y + 1 },
					newTris, mainData.textureDatas[(size_t)TextureID::water]);
			}
		}
	}

	if (pos.y == 255)
	{
		for (int8_t x = height + 1; x < 1; x++)
		{
			if (x == 0)
			{
				createQuad(
					{ (double)pos.x, (double)x, (double)255.9998 },
					{ (double)pos.x + 1, (double)x, (double)255.9998 },
					{ (double)pos.x, (double)x - 1, (double)255.9998 },
					{ (double)pos.x + 1, (double)x - 1, (double)255.9998 },
					newTris, mainData.textureDatas[(size_t)TextureID::waterSide]);
			}
			else
			{
				createQuad(
					{ (double)pos.x, (double)x, (double)255.9998 },
					{ (double)pos.x + 1, (double)x, (double)255.9998 },
					{ (double)pos.x, (double)x - 1, (double)255.9998 },
					{ (double)pos.x + 1, (double)x - 1, (double)255.9998 },
					newTris, mainData.textureDatas[(size_t)TextureID::water]);
			}
		}
	}

	// Land Occ

	switch (tile.landOcc.type)
	{
	case LandOccEnum::debug:
	{
		double buildHeight = height > 0 ? (double)height : 0;
		createQuad(
			{ (double)pos.x, buildHeight + 1, (double)pos.y + 0.5 },
			{ (double)pos.x + 1, buildHeight + 1, (double)pos.y + 0.5 },
			{ (double)pos.x, buildHeight, (double)pos.y + 0.5 },
			{ (double)pos.x + 1, buildHeight, (double)pos.y + 0.5 },
			newTris, mainData.textureDatas[(size_t)TextureID::debug]);
		break;
	}
	case LandOccEnum::foliage:
	{
		double buildHeight = (double)height;
		createQuad(
			{ (double)pos.x, buildHeight + 1, (double)pos.y + 0.5 },
			{ (double)pos.x + 1, buildHeight + 1, (double)pos.y + 0.5 },
			{ (double)pos.x, buildHeight, (double)pos.y + 0.5 },
			{ (double)pos.x + 1, buildHeight, (double)pos.y + 0.5 },
			newTris, mainData.textureDatas[(size_t)(mainData.foliageDatas[*(uint8_t*)&tile.landOcc.data].texture)]);
		break;
	}
	}

	// End

	tile.trisPosSize = appendTris(mainData, newTris);
}

uint64_t getSystemTime()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void generateMap(MainData& mainData, Map& map, MapTerrainType mapTerrainType, uint64_t seed)
{
	srand(seed);
	mainData.redrawMap = 1;
	uint8_t x = 0;
	do
	{
		uint8_t y = 0;
		do
		{
			switch (mapTerrainType)
			{
			case MapTerrainType::regular:
			{
				double elevation = 0;

				for (uint8_t z = 1; z < 8; z++)
				{
					uint8_t c = pow(2, z);
					uint8_t x1 = floor(x / (256. / c));
					uint8_t y1 = floor(y / (256. / c));
					elevation += filter({ (double)x, (double)y }, { x1 * (256. / c), y1 * (256. / c) }, (256. / c), random(seed, x1, y1, z) / (c * 2), random(seed, x1 + 1, y1, z) / (c * 2), random(seed, x1, y1 + 1, z) / (c * 2), random(seed, x1 + 1, y1 + 1, z) / (c * 2)) - (256 / (c * 4));
				}

				int8_t elevation1 = (int8_t)elevation;
				map[x][y].height = elevation1;

				GroundMaterialEnum groundMaterial = GroundMaterialEnum::grass;

				if (elevation1 < 1)
				{
					groundMaterial = GroundMaterialEnum::sand;
				}
				if (elevation1 > 20)
				{
					groundMaterial = GroundMaterialEnum::rock;
				}
				if (elevation1 > 25)
				{
					groundMaterial = GroundMaterialEnum::snow;
				}
				map[x][y].groundMaterial = groundMaterial;
				map[x][y].landOcc.type = LandOccEnum::none;
				//random(seed, x1, y1, z)
				/*if (random(seed, x, y, 10) == 0)
				{
					map[x][y].landOcc.type = LandOccEnum::debug;
				}*/
				if (elevation1 > 1 && elevation1 < 20)
				{
					if (random(seed, x, y, 10) == 0)
					{
						map[x][y].landOcc.type = LandOccEnum::foliage;
						*(FoliageEnum*)(&map[x][y].landOcc.data) = FoliageEnum::pineTree;
					}
				}
				if (elevation1 < 1)
				{
					if (random(seed, x, y, 10) == 0)
					{
						map[x][y].landOcc.type = LandOccEnum::foliage;
						*(FoliageEnum*)(&map[x][y].landOcc.data) = FoliageEnum::rockPile;
					}
				}

				break;
			}
			case MapTerrainType::swamp:
			{
				bool height = rand() % 2;
				map[x][y].groundMaterial = height ? GroundMaterialEnum::grass : GroundMaterialEnum::sand;
				map[x][y].height = height - 1;
				map[x][y].landOcc.type = LandOccEnum::none;
				break;
			}
			case MapTerrainType::flatGrass:
				map[x][y].groundMaterial = GroundMaterialEnum::grass;
				map[x][y].height = 0;
				map[x][y].landOcc.type = LandOccEnum::none;
				break;
			case MapTerrainType::flatWater:
				map[x][y].groundMaterial = GroundMaterialEnum::sand;
				map[x][y].height = -1;
				map[x][y].landOcc.type = LandOccEnum::none;
				break;
			}
			y++;
		} while (y != 0);
		x++;
	} while (x != 0);
}