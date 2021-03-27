#include "SFML/Graphics.hpp"

int WinMain()
{
    uint64_t time = 0;

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "TownBig");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        time++;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        sf::CircleShape shape(500.f);
        shape.setPointCount(100);
        shape.setPosition(0, 0);
        shape.setFillColor(sf::Color::Green);
        window.draw(shape);

        shape = sf::CircleShape(50.f);
        shape.setPosition(10, 10);
        shape.setFillColor(sf::Color::Red);
        window.draw(shape);

        sf::RectangleShape rShape;
        rShape.setPosition(20, time % 100);
        rShape.setSize(sf::Vector2f(100, 50));
        rShape.setFillColor(sf::Color::Blue);
        window.draw(rShape);

        window.display();
    }

    return 0;
}