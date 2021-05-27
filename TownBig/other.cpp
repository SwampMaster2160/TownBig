#include "main.hpp"

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

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(1);
	window.setActive(1);
}

PosSize appendTris(MainData& mainData, std::vector<TriPoint>& newTris)
{
	size_t triVectorSize = mainData.triangles.size();
	mainData.triangles.resize(triVectorSize + newTris.size());
	memcpy(&mainData.triangles[triVectorSize], &newTris[0], newTris.size() * sizeof(TriPoint));
	return { triVectorSize, newTris.size() };
}

void deleteTris(MainData& mainData, PosSize posSize)
{
	for (uint64_t x = posSize.pos; x < posSize.getEnd(); x++)
	{
		for (uint64_t y = 0; y < 3; y++)
		{
			mainData.triangles[x].doubles[y] = 0;
		}
	}
}

void drawTile(MainData& mainData, Tile& tile, uint8_t x, uint8_t y)
{
	if (!mainData.redrawMap)
	{
		deleteTris(mainData, tile.trisPosSize);
	}

	uint8_t type = tile.type;
	std::vector<TriPoint> newTris = {};
	newTris.push_back({ (double)x, 0, (double)y, 0, 0 });
	newTris.push_back({ (double)x + 1, 0, (double)y, 1 / ((double)type + 1), 0 });
	newTris.push_back({ (double)x + 1, 0, (double)y + 1, 1, 1 });
	newTris.push_back({ (double)x, 0, (double)y, 0, 0 });
	newTris.push_back({ (double)x + 1, 0, (double)y + 1, 1, 1 });
	newTris.push_back({ (double)x, 0, (double)y + 1, 0, 1 });
	appendTris(mainData, newTris);
}