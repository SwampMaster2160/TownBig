#include "main.hpp"
#include <iostream>

const Version currentVersion = {0, 0, 0, 0};

int main()
{
    std::cout << "Version: " << currentVersion.major << '.' << currentVersion.mid << '.' << currentVersion.minor << '.' << currentVersion.snapshot << '\n';
    uint64_t time = 0;

    // Init Window
    sf::Vector2u windowedWindowSize;
    sf::RenderWindow window(sf::VideoMode(256, 256), "TownBig");
    WindowData windowData = {};
    windowData.fullScreen = 0;
    initWindow(window, windowData, 1);

    while (window.isOpen())
    {
        time++;

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
            }
        }

        window.clear();

        sf::RectangleShape rShape;
        rShape.setPosition(20, time % 100);
        rShape.setSize(sf::Vector2f(100, 50));
        rShape.setFillColor(sf::Color::Blue);
        window.draw(rShape);

        window.display();
    }

    return 0;
}
