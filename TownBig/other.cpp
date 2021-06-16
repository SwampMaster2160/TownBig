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
	newTris.push_back({ pos3.x, pos3.y, pos3.z, textureData.xEnd,   textureData.yEnd   });
	newTris.push_back({ pos0.x, pos0.y, pos0.z, textureData.xStart, textureData.yStart });
	newTris.push_back({ pos3.x, pos3.y, pos3.z, textureData.xEnd,   textureData.yEnd   });
	newTris.push_back({ pos2.x, pos2.y, pos2.z, textureData.xStart, textureData.yEnd   });
}

bool isTileInMap(sf::Vector2<int16_t> pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.x > 255 || pos.y > 255) return 0;
	return 1;
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

	uint8_t type = (uint8_t)tile.groundMaterial * 2;
	int8_t height = tile.height;
	TextureData& textureData = mainData.textureDatas[type];
	std::vector<TriPoint> newTris = {};

	// Terrain

	createQuad(
		{ (double)pos.x, (double)height, (double)pos.y },
		{ (double)pos.x + 1, (double)height, (double)pos.y },
		{ (double)pos.x, (double)height, (double)pos.y + 1 },
		{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
		newTris, textureData);

	// Slopes

	bool drawSlope = 1;

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
		}
		if (neighborHeights[2][1] == height + 1)
		{
			createQuad(
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
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
		}
		if (neighborHeights[0][1] == height + 1)
		{
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y },
				newTris, textureData);
		}

		if (neighborHeights[1][0] == height + 1)
		{
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
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x + 1, (double)height, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				newTris, textureData);
		}
		if (neighborHeights[1][2] == height + 1)
		{
			createQuad(
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height + 1, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y + 1 },
				{ (double)pos.x, (double)height, (double)pos.y },
				newTris, textureData);
		}
		if (neighborHeights[0][1] == height + 1)
		{
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

	tile.trisPosSize = appendTris(mainData, newTris);
}

uint64_t getSystemTime()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}