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
	contextSettings.antialiasingLevel = 4;

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

void drawTile(MainData& mainData, Tile& tile, sf::Vector2<uint8_t> pos)
{
	if (!mainData.redrawMap)
	{
		deleteTris(mainData, tile.trisPosSize);
	}

	uint8_t type = tile.type;
	std::vector<TriPoint> newTris = {};
	newTris.push_back({ (double)pos.x, 0, (double)pos.y, 0, 0 });
	newTris.push_back({ (double)pos.x + 1, 0, (double)pos.y, 1 / ((double)type + 1), 0 });
	newTris.push_back({ (double)pos.x + 1, 0, (double)pos.y + 1, 1, 1 });
	newTris.push_back({ (double)pos.x, 0, (double)pos.y, 0, 0 });
	newTris.push_back({ (double)pos.x + 1, 0, (double)pos.y + 1, 1, 1 });
	newTris.push_back({ (double)pos.x, 0, (double)pos.y + 1, 0, 1 });
	if (tile.type == 1)
	{
		newTris.push_back({ (double)pos.x, 2, (double)pos.y, 0, 0 });
		newTris.push_back({ (double)pos.x + 1, 2, (double)pos.y + 1, 0, 1 });
		newTris.push_back({ (double)pos.x, 2, (double)pos.y + 1, 0, 1 });
	}
	tile.trisPosSize = appendTris(mainData, newTris);
}

uint64_t getSystemTime()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}