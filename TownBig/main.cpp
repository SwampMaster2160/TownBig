#include "main.hpp"
#include <iostream>

const Version currentVersion = {2, 0, 0, 0};

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

    sf::Vector2<double> cameraPos = {0, 128};
    uint8_t cameraZoom = 2;

    // Graphics

    sf::Texture textures;
    textures.setSrgb(false);
    textures.loadFromFile("assets/textures/RGB.png");

    std::vector<TriPoint> triangles = {};

    uint8_t x = 0;
    do
    {
        uint8_t y = 0;
        do
        {
            triangles.push_back({ (double)x, 0, (double)y, 0, 0 });
            triangles.push_back({ (double)x + 1, 0, (double)y, 1, 0 });
            triangles.push_back({ (double)x + 1, 0, (double)y + 1, 1, 1 });
            triangles.push_back({ (double)x, 0, (double)y, 0, 0 });
            triangles.push_back({ (double)x + 1, 0, (double)y + 1, 1, 1 });
            triangles.push_back({ (double)x, 0, (double)y + 1, 0, 1 });
            y++;
        } while (y != 0);
        x++;
    } while (x != 0);

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

        cameraPos = {sin((double)time / 10.), cos((double)time / 10.) + 128. };

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
        sf::Texture::bind(&textures);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0., 0.75, 1., 0.);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        GLdouble* t1 = (GLdouble*)&triangles[0];
        glVertexPointer(3, GL_DOUBLE, 8 * sizeof(GLdouble), t1);
        glTexCoordPointer(2, GL_DOUBLE, 8 * sizeof(GLdouble), t1 + 4);
        glDrawArrays(GL_TRIANGLES, 0, triangles.size());

        window.display();

        // Loop End

        for (uint8_t x = 0; x < (uint8_t)Inputs::size; x++) inputsLast[x] = inputs[x];
    }

    return 0;
}
