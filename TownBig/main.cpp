#include "main.hpp"
#include <iostream>

const Version currentVersion = {6, 0, 0, 0};

int main()
{
    MainData mainData;
    std::cout << "Version: " << currentVersion.major << '.' << currentVersion.mid << '.' << currentVersion.minor << '.' << currentVersion.snapshot << '\n';
    uint64_t time = 0;
    bool inputsLast[(uint8_t)Inputs::size];
    uint64_t lastSystemTime = getSystemTime();
    for (uint8_t x = 0; x < (uint8_t)Inputs::size; x++) inputsLast[x] = 0;

    // Init Window

    sf::Vector2u windowedWindowSize;
    sf::RenderWindow window(sf::VideoMode(256, 256), "TownBig");
    mainData.fullScreen = 0;
    initWindow(window, mainData, 1);

    // Camera

    sf::Vector2<double> cameraPos = {128, 128};
    uint8_t cameraZoom = 4;

    // Graphics

    sf::Image image;
    image.create(4096, 4096, sf::Color(0, 0, 0, 0));

    sf::Image subImage;

    subImage.loadFromFile("assets/textures/grass.png");
    image.copy(subImage, 0, 0, sf::IntRect(0, 0, 16, 16));
    subImage.loadFromFile("assets/textures/water.png");
    image.copy(subImage, 16, 0, sf::IntRect(0, 0, 16, 16));

    sf::Texture textures;
    textures.setSrgb(false);
    textures.loadFromImage(image);

    // Map

    mainData.redrawMap = 1;
    std::vector<sf::Vector2<uint8_t>> redrawTileQueue;
    Map& map = *(new Map);


    {
        uint8_t x = 0;
        do
        {
            uint8_t y = 0;
            do
            {
                map[x][y].type = rand() % 2;
                y++;
            } while (y != 0);
            x++;
        } while (x != 0);
    }

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
                inputs[(uint8_t)Inputs::windowResize] = 1;
                break;
            case sf::Event::MouseWheelMoved:
                inputs[(uint8_t)Inputs::zoomIn] = event.mouseWheel.delta > 0;
                inputs[(uint8_t)Inputs::zoomOut] = event.mouseWheel.delta < 0;
                break;
            }
        }

        // Get inputs

        inputs[(uint8_t)Inputs::fullScreen] = sf::Keyboard::isKeyPressed(sf::Keyboard::F11);
        inputs[(uint8_t)Inputs::mainClick] = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        inputs[(uint8_t)Inputs::pan] = sf::Mouse::isButtonPressed(sf::Mouse::Right);

        uint64_t systemTime = getSystemTime();
        double secondsPerFrame = (double)(systemTime - lastSystemTime) / 1000000000.;

        if (inputs[(uint8_t)Inputs::fullScreen] & !inputsLast[(uint8_t)Inputs::fullScreen])
        {
            mainData.fullScreen = !mainData.fullScreen;
            initWindow(window, mainData, 0);
        }
        if (inputs[(uint8_t)Inputs::windowResize])
        {
            mainData.windowedWindowSize = window.getSize();
            initWindow(window, mainData, 0);
        }

        // Zoom

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2u windowSize = window.getSize();

        if (inputs[(uint8_t)Inputs::zoomIn]) cameraZoom -= 1;
        if (inputs[(uint8_t)Inputs::zoomOut]) cameraZoom += 1;
        if (cameraZoom < 1) cameraZoom = 1;
        if (cameraZoom > 7) cameraZoom = 7;

        double cameraZoomD = pow(2., cameraZoom);

        // Pan

        if (inputs[(uint8_t)Inputs::pan])
        {
            cameraPos += { ((double)mousePos.x / (double)windowSize.x - 0.5) * secondsPerFrame * 5 * cameraZoomD, ((double)mousePos.y / (double)windowSize.y - 0.5) * secondsPerFrame * 5 * cameraZoomD };
        }
        if (cameraPos.x < 0) cameraPos.x = 0;
        if (cameraPos.y < 0) cameraPos.y = 0;
        if (cameraPos.x > 256) cameraPos.x = 256;
        if (cameraPos.y > 256) cameraPos.y = 256;

        // Game logic

        for (uint8_t x = 0; x < 10; x++)
        {
            sf::Vector2<uint8_t> pos = { (uint8_t)rand() , (uint8_t)rand() };
            map[pos.x][pos.y].type = rand() % 2;
            redrawTileQueue.push_back(pos);
        }

        // Render

        for (; redrawTileQueue.size() > 0;)
        {
            sf::Vector2<uint8_t> pos = redrawTileQueue[redrawTileQueue.size() - 1];
            drawTile(mainData, map[pos.x][pos.y], pos);
            redrawTileQueue.pop_back();
        }

        if (mainData.redrawMap)
        {
            mainData.triangles = {};
            mainData.freeTriangles = {};
            uint8_t x = 0;
            do
            {
                uint8_t y = 0;
                do
                {
                    drawTile(mainData, map[x][y], { x, y });
                    y++;
                } while (y != 0);
                x++;
            } while (x != 0);
            mainData.redrawMap = 0;
        }

        cameraZoomD = pow(2., -cameraZoom);

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

        GLdouble* t1 = (GLdouble*)&mainData.triangles[0];
        glVertexPointer(3, GL_DOUBLE, 8 * sizeof(GLdouble), t1);
        glTexCoordPointer(2, GL_DOUBLE, 8 * sizeof(GLdouble), t1 + 4);
        glDrawArrays(GL_TRIANGLES, 0, mainData.triangles.size());

        window.display();

        // Loop End

        for (uint8_t x = 0; x < (uint8_t)Inputs::size; x++) inputsLast[x] = inputs[x];
        lastSystemTime = systemTime;
    }

    delete &map;
    return 0;
}
