#include "main.hpp"
#include <iostream>

const Version currentVersion = {1, 0, 0, 0};

int main()
{
    std::cout << "Version: " << currentVersion.major << '.' << currentVersion.mid << '.' << currentVersion.minor << '.' << currentVersion.snapshot << '\n';
    uint64_t time = 0;
    bool inputsLast[(uint8_t)Inputs::size];
    for (uint8_t x = 0; x < (uint8_t)Inputs::size; x++) inputsLast[x] = 0;

    // Init Window

    sf::Vector2u windowedWindowSize;
    sf::RenderWindow window(sf::VideoMode(256, 256), "TownBig");
    WindowData windowData = {};
    windowData.fullScreen = 0;
    initWindow(window, windowData, 1);

    // Camera

    sf::Vector2<double> cameraPos;
    uint8_t cameraZoom = 4;

    // Main game loop

    while (window.isOpen())
    {
        time++;

        // Clear input

        bool inputs[(uint8_t)Inputs::size];
        for (uint8_t x = 0; x < (uint8_t)Inputs::size; x++) inputs[x] = 0;

        // Poll events

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::Resized:
                break;
            }
        }

        // Game logic

        // Get inputs
        inputs[(uint8_t)Inputs::fullScreen] = sf::Keyboard::isKeyPressed(sf::Keyboard::F11);

        if (inputs[(uint8_t)Inputs::fullScreen] & !inputsLast[(uint8_t)Inputs::fullScreen])
        {
            windowData.fullScreen = !windowData.fullScreen;
            initWindow(window, windowData, 0);
        }

        // Render

        //double cameraEdgeClipAmount = 4 / cameraZoomD;
        double cameraZoomD = pow(2., -cameraZoom);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslated(0., 0., -200.);
        glRotated(45., 1., 0., 0.);
        glRotated(45., 0., 1., 0.);
        glTranslated(-128 * cameraZoomD, 0., 128 * cameraZoomD);
        glTranslated((-cameraPos.x * 1) * cameraZoomD, 0., (-cameraPos.x * 1) * cameraZoomD);
        glTranslated((cameraPos.y * 1) * cameraZoomD, 0., (-cameraPos.y * 1) * cameraZoomD);
        glScaled(cameraZoomD, cameraZoomD, cameraZoomD);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*sf::RectangleShape rShape;
        rShape.setPosition(20, time % 100);
        rShape.setSize(sf::Vector2f(100, 50));
        rShape.setFillColor(sf::Color::Blue);
        window.draw(rShape);*/

        window.display();

        // Loop End

        for (uint8_t x = 0; x < (uint8_t)Inputs::size; x++) inputsLast[x] = inputs[x];
    }

    return 0;
}
