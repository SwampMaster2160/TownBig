#include "main.hpp"

void initWindow(sf::RenderWindow& window, WindowData& windowData, bool firstTime)
{
	sf::VideoMode mode = sf::VideoMode::getDesktopMode();
	sf::Vector2u screenSize = { mode.width, mode.height };
	if (firstTime) windowData.windowedWindowSize = { screenSize.x / 2, screenSize.y / 2 };

	if (windowData.fullScreen)
	{
		window.create({ screenSize.x, screenSize.y }, "TownBig", sf::Style::Fullscreen);
	}
	else
	{
		window.create({ windowData.windowedWindowSize.x, windowData.windowedWindowSize.y }, "TownBig", sf::Style::Default);
	}

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(1);
	window.setActive(1);
}