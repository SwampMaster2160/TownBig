#include "main.hpp"
#include <string>
#include <cstring>
#include <iostream>
#include <filesystem>

const Version currentVersion = {3, 1, 0, 0};

int main()
{
    MainData mainData;
    std::cout << "Version: " << currentVersion.major << '.' << currentVersion.mid << '.' << currentVersion.minor << '.' << currentVersion.snapshot;

    if (currentVersion.snapshot != 0xFFFF) std::cout << " (Snapshop)";

    std::cout << '\n';

    uint64_t time = 0;
    bool inputsLast[(uint8_t)Inputs::size];
    uint64_t lastSystemTime = getSystemTime();
    for (uint8_t x = 0; x < (uint8_t)Inputs::size; x++) inputsLast[x] = 0;

    //std::cout << textureNames[(uint8_t)TextureID::grass] << '\n';

    // Init Window

    sf::Vector2u windowedWindowSize;
    sf::RenderWindow window(sf::VideoMode(256, 256), "TownBig");
    mainData.fullScreen = 1;
    initWindow(window, mainData, 1);

    // Camera

    sf::Vector2<double> cameraPos = {128, 128};
    uint8_t cameraZoom = 4;

    // Graphics

    mainData.textureDatas = {};

    for (size_t x = 0; x < (size_t)TextureID::size; x++)
    {
        TextureData data = {};
        data.name = textureNames[x];
        mainData.textureDatas.push_back(data);
        //std::cout << textureNames[x] << '\n';
    }

    sf::Image image;
    image.create(4096, 4096, sf::Color(0, 0, 0, 0));

    sf::Image subImage;

    size_t x = 0;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator("assets/textures"))
    {
        std::string string = entry.path().filename().generic_string();
        subImage.loadFromFile(entry.path().generic_string());
        image.copy(subImage, (x % 256) * 16, (x / 256) * 16, sf::IntRect(0, 0, 16, 16));

        size_t lengthNoExt = (size_t)strrchr(string.c_str(), '.') - (size_t)string.c_str();

        for (size_t y = 0; y < (size_t)TextureID::size; y++)
        {
            //std::cout << string << ' ' << lengthNoExt << ' ' << strncmp(string.c_str(), textureDatas[y].name, lengthNoExt) << '\n';
            if (strncmp(string.c_str(), mainData.textureDatas[y].name, lengthNoExt) == 0)
            {
                mainData.textureDatas[y].xStart = 0.00390625 * (x % 256);
                mainData.textureDatas[y].yStart = 0.00390625 * (x / 256);
                mainData.textureDatas[y].xEnd = 0.00390625 * (x % 256) + 0.00390625;
                mainData.textureDatas[y].yEnd = 0.00390625 * (x / 256) + 0.00390625;
            }
        }

        //std::cout << string << '\n';

        x++;
    }

    sf::Texture textureMap;
    textureMap.setSrgb(false);
    textureMap.loadFromImage(image);

    // Map

    std::vector<sf::Vector2<uint8_t>> redrawTileQueue;
    Map& map = *(new Map);

    generateMap(mainData, map, MapTerrainType::swamp, rand());

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
        inputs[(uint8_t)Inputs::f1] = sf::Keyboard::isKeyPressed(sf::Keyboard::F1);
        inputs[(uint8_t)Inputs::f2] = sf::Keyboard::isKeyPressed(sf::Keyboard::F2);
        inputs[(uint8_t)Inputs::f3] = sf::Keyboard::isKeyPressed(sf::Keyboard::F3);
        inputs[(uint8_t)Inputs::f4] = sf::Keyboard::isKeyPressed(sf::Keyboard::F4);
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

        /*for (uint8_t x = 0; x < 10; x++)
        {
            sf::Vector2<uint8_t> pos = { (uint8_t)rand() , (uint8_t)rand() };
            map[pos.x][pos.y].groundMaterial = (GroundMaterial)(rand() % 2);
            redrawTileQueue.push_back(pos);
        }*/

        if (inputs[(uint8_t)Inputs::f1]) generateMap(mainData, map, MapTerrainType::regular, rand());
        if (inputs[(uint8_t)Inputs::f2]) generateMap(mainData, map, MapTerrainType::swamp, rand());
        if (inputs[(uint8_t)Inputs::f3]) generateMap(mainData, map, MapTerrainType::flatGrass, rand());
        if (inputs[(uint8_t)Inputs::f4]) generateMap(mainData, map, MapTerrainType::flatWater, rand());

        // Render

        for (; redrawTileQueue.size() > 0;)
        {
            sf::Vector2<uint8_t> pos = redrawTileQueue[redrawTileQueue.size() - 1];
            drawTile(mainData, map[pos.x][pos.y], pos, map);
            redrawTileQueue.pop_back();
        }

        if (mainData.redrawMap)
        {
            mainData.triangles = {};
            mainData.freeTriangles = {};
            uint8_t x = 255;
            do
            {
                uint8_t y = 0;
                do
                {
                    drawTile(mainData, map[x][y], { x, y }, map);
                    y++;
                } while (y != 0);
                x--;
            } while (x != 255);
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
        sf::Texture::bind(&textureMap);

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
