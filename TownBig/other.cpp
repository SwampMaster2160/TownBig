#include "main.hpp"
#include <iostream>
#include <chrono>

sf::Vector2f toVec2(ScreenPos screenPos, MainData& mainData)
{
	float x = ((uint8_t)screenPos.hAlign) * mainData.windowSize.x / 2. + screenPos.pos.x * mainData.guiScale;
	float y = ((uint8_t)screenPos.vAlign) * mainData.windowSize.y / 2. + screenPos.pos.y * mainData.guiScale;
	return { x, y };
}

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

	windowData.windowSize = window.getSize();
	windowData.guiScale = windowData.windowSize.y / 256;

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

PosSize appendTris(MainData& mainData, std::vector<TriPoint>& newTris, Chunk& chunk)
{
	//std::cout << "a\n";
	for (size_t x = 0; x < chunk.freeTriangles.size(); x++)
	{
		if (chunk.freeTriangles[x].size >= newTris.size())
		{
			PosSize out = { chunk.freeTriangles[x].pos, newTris.size() };
 			memcpy(&chunk.triangles[chunk.freeTriangles[x].pos], &newTris[0], newTris.size() * sizeof(TriPoint));
			chunk.freeTriangles[x].pos += newTris.size();
			chunk.freeTriangles[x].size -= newTris.size();

			if (chunk.freeTriangles[x].size == 0)
			{
				chunk.freeTriangles.erase(chunk.freeTriangles.begin() + x);
			}

			return out;
		}
	}

	size_t triVectorSize = chunk.triangles.size();
	chunk.triangles.resize(triVectorSize + newTris.size());
	memcpy(&chunk.triangles[triVectorSize], &newTris[0], newTris.size() * sizeof(TriPoint));
	return { triVectorSize, newTris.size() };
}

void deleteTris(MainData& mainData, PosSize posSize, Chunk& chunk)
{
	for (size_t x = posSize.pos; x < posSize.getEnd(); x++)
	{
		for (size_t y = 0; y < 3; y++)
		{
			chunk.triangles[x].doubles[y] = 0;
		}
	}
	chunk.freeTriangles.push_back(posSize);

	for (size_t x = 0; x < chunk.freeTriangles.size(); x++)
	{
		if (posSize.pos == chunk.freeTriangles[x].getEnd())
		{
			chunk.freeTriangles[chunk.freeTriangles.size() - 1].pos -= chunk.freeTriangles[x].size;
			chunk.freeTriangles[chunk.freeTriangles.size() - 1].size += chunk.freeTriangles[x].size;
			chunk.freeTriangles.erase(chunk.freeTriangles.begin() + x);
		}
	}

	for (size_t x = 0; x < chunk.freeTriangles.size(); x++)
	{
		if (posSize.getEnd() == chunk.freeTriangles[x].pos)
		{
			chunk.freeTriangles[chunk.freeTriangles.size() - 1].size += chunk.freeTriangles[x].size;
			chunk.freeTriangles.erase(chunk.freeTriangles.begin() + x);
		}
	}
}

void createQuad(sf::Vector3<double> pos0, sf::Vector3<double> pos1, sf::Vector3<double> pos2, sf::Vector3<double> pos3, std::vector<TriPoint>& newTris, TextureData& textureData, Chunk& chunk)
{
	newTris.push_back({ pos0.x + chunk.pos.x * 256, pos0.y, pos0.z + chunk.pos.y * 256, textureData.xStart, textureData.yStart });
	newTris.push_back({ pos1.x + chunk.pos.x * 256, pos1.y, pos1.z + chunk.pos.y * 256, textureData.xEnd,   textureData.yStart });
	newTris.push_back({ pos3.x + chunk.pos.x * 256, pos3.y, pos3.z + chunk.pos.y * 256, textureData.xEnd,   textureData.yEnd });
	newTris.push_back({ pos0.x + chunk.pos.x * 256, pos0.y, pos0.z + chunk.pos.y * 256, textureData.xStart, textureData.yStart });
	newTris.push_back({ pos3.x + chunk.pos.x * 256, pos3.y, pos3.z + chunk.pos.y * 256, textureData.xEnd,   textureData.yEnd });
	newTris.push_back({ pos2.x + chunk.pos.x * 256, pos2.y, pos2.z + chunk.pos.y * 256, textureData.xStart, textureData.yEnd });
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

void drawTile(MainData& mainData, Tile& tile, sf::Vector2<uint64_t> pos, Chunk& chunk)
{
	if (!chunk.redrawAll)
	{
		deleteTris(mainData, tile.trisPosSize, chunk);
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
				neighborHeights[x][y] = chunk.tiles[xMap + yMap * 256].height;
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
		newTris, textureData, chunk);

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
				newTris, textureData, chunk);
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData, chunk);
		}
		if (neighborHeights[2][1] == height + 1)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData, chunk);
			createQuad(
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 0.9999 },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 0.9999 },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 0.9999 },
				{ (double)pos.x, (double)height, (double)pos.y + 0.9999 },
				newTris, textureData, chunk);
		}
		if (neighborHeights[1][2] == height + 1)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y },
				newTris, textureData, chunk);
			createQuad(
				{ (double)pos.x + 0.0001, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x + 0.0001, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x + 0.0001, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 0.0001, (double)height, (double)pos.y },
				newTris, textureData, chunk);
		}
		if (neighborHeights[0][1] == height + 1)
		{
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				newTris, textureData, chunk);
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				newTris, textureData, chunk);
		}

		if (neighborHeights[0][0] == height + 1 && neighborHeights[0][1] == height && neighborHeights[1][0] == height)
		{
			createQuad(
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				newTris, textureData, chunk);
		}
		if (neighborHeights[2][0] == height + 1 && neighborHeights[1][0] == height && neighborHeights[2][1] == height)
		{
			createQuad(
				{ (double)pos.x, (double)height, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				newTris, textureData, chunk);
		}
		if (neighborHeights[2][2] == height + 1 && neighborHeights[2][1] == height && neighborHeights[1][2] == height)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData, chunk);
		}
		if (neighborHeights[2][2] == height + 1 && neighborHeights[2][1] == height && neighborHeights[1][2] == height)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData, chunk);
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
			newTris, textureData, chunk);
	}
	for (int8_t x = neighborHeights[1][2]; x <= height; x++)
	{
		createQuad(
			{ (double)pos.x, (double)x, (double)pos.y + 1 },
			{ (double)pos.x + 1, (double)x, (double)pos.y + 1 },
			{ (double)pos.x, (double)x - 1, (double)pos.y + 1 },
			{ (double)pos.x + 1, (double)x - 1, (double)pos.y + 1 },
			newTris, textureData, chunk);
	}

	// Water

	if (height < 0)
	{
		createQuad(
			{ (double)pos.x, -0.0625, (double)pos.y },
			{ (double)pos.x + 1, -0.0625, (double)pos.y },
			{ (double)pos.x, -0.0625, (double)pos.y + 1 },
			{ (double)pos.x + 1, -0.0625, (double)pos.y + 1 },
			newTris, mainData.textureDatas[(size_t)TextureID::water], chunk);
	}

	/*if (pos.x == 0)
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
	}*/

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
			newTris, mainData.textureDatas[(size_t)TextureID::debug], chunk);
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
			newTris, mainData.textureDatas[(size_t)(mainData.foliageDatas[*(uint8_t*)&tile.landOcc.data].texture)], chunk);
		break;
	}
	}

	// End

	tile.trisPosSize = appendTris(mainData, newTris, chunk);
}

void renderChunk(MainData& mainData, Chunk& chunk)
{
	if (chunk.redrawAll)
	{
		chunk.triangles = {};
		chunk.freeTriangles = {};
		uint8_t x = 255;
		do
		{
			uint8_t y = 0;
			do
			{
				drawTile(mainData, chunk.tiles[x + y * 256], { x, y }, chunk);
				y++;
			} while (y != 0);
			x--;
		} while (x != 255);
		chunk.redrawAll = 0;
	}
}

uint64_t getSystemTime()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void generateMap(MainData& mainData, Chunk& chunk, MapTerrainType mapTerrainType, uint64_t seed)
{
	srand(seed);
	chunk.redrawAll = 1;
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
				chunk.tiles[x + y * 256].height = elevation1;

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
				chunk.tiles[x + y * 256].groundMaterial = groundMaterial;
				chunk.tiles[x + y * 256].landOcc.type = LandOccEnum::none;
				//random(seed, x1, y1, z)
				/*if (random(seed, x, y, 10) == 0)
				{
					map[x][y].landOcc.type = LandOccEnum::debug;
				}*/
				if (elevation1 > 1 && elevation1 < 20)
				{
					if (random(seed, x, y, 10) == 0)
					{
						chunk.tiles[x + y * 256].landOcc.type = LandOccEnum::foliage;
						*(FoliageEnum*)(&chunk.tiles[x + y * 256].landOcc.data) = random(seed, x, y, 11) < 128 ? FoliageEnum::pineTree : FoliageEnum::oakTree;
					}
				}
				if (elevation1 < 1 || elevation1 > 20 && elevation1 < 25)
				{
					if (random(seed, x, y, 10) == 0)
					{
						chunk.tiles[x + y * 256].landOcc.type = LandOccEnum::foliage;
						*(FoliageEnum*)(&chunk.tiles[x + y * 256].landOcc.data) = FoliageEnum::rockPile;
					}
				}

				break;
			}
			case MapTerrainType::swamp:
			{
				bool height = rand() % 2;
				chunk.tiles[x + y * 256].groundMaterial = height ? GroundMaterialEnum::grass : GroundMaterialEnum::sand;
				chunk.tiles[x + y * 256].height = height - 1;
				chunk.tiles[x + y * 256].landOcc.type = LandOccEnum::none;
				break;
			}
			case MapTerrainType::flatGrass:
				chunk.tiles[x + y * 256].groundMaterial = GroundMaterialEnum::grass;
				chunk.tiles[x + y * 256].height = 0;
				chunk.tiles[x + y * 256].landOcc.type = LandOccEnum::none;
				break;
			case MapTerrainType::flatWater:
				chunk.tiles[x + y * 256].groundMaterial = GroundMaterialEnum::sand;
				chunk.tiles[x + y * 256].height = -1;
				chunk.tiles[x + y * 256].landOcc.type = LandOccEnum::none;
				break;
			}
			y++;
		} while (y != 0);
		x++;
	} while (x != 0);
}

void renderRect(sf::RenderWindow& window, MainData& mainData, ScreenPos pos, ScreenPos end, sf::Color color)
{
	sf::RectangleShape rect;
	rect.setPosition(toVec2(pos, mainData));
	rect.setSize(toVec2(end, mainData) - toVec2(pos, mainData));
	rect.setFillColor(color);
	window.draw(rect);
}