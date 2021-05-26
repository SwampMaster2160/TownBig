#include "main.hpp"

void initWindow(sf::RenderWindow& window, WindowData& windowData, bool firstTime)
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