#include "main.hpp"
#include <string>
#include <cstring>
#include <iostream>
#include <filesystem>

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

    mainData.groundMaterialDatas[(size_t)GroundMaterialEnum::grass].texture = TextureID::grass;
    mainData.groundMaterialDatas[(size_t)GroundMaterialEnum::sand].texture = TextureID::sand;
    mainData.groundMaterialDatas[(size_t)GroundMaterialEnum::rock].texture = TextureID::rock;
    mainData.groundMaterialDatas[(size_t)GroundMaterialEnum::snow].texture = TextureID::snow;

    mainData.foliageDatas[(size_t)FoliageEnum::pineTree].texture = TextureID::pineTree;
    mainData.foliageDatas[(size_t)FoliageEnum::rockPile].texture = TextureID::rockPile;
    mainData.foliageDatas[(size_t)FoliageEnum::oakTree].texture = TextureID::oakTree;

    // GUI

    mainData.currentGUI = GUIEnum::ingame;

    mainData.guis[(size_t)GUIEnum::ingame] = {};
    mainData.guis[(size_t)GUIEnum::ingame].elements.push_back({ GUIElementEnum::rect, nullptr });

    // Map

    //std::vector<sf::Vector2<uint64_t>> redrawTileQueue;
    uint8_t chunkLayout[2][2];
    for (uint8_t x = 0; x < 4; x++)
    {
        chunkLayout[x % 2][x / 2] = x;
    }
    mainData.chunks = new Chunk[4];

    for (uint8_t x = 0; x < 4; x++)//4
    {
        generateMap(mainData, mainData.chunks[x], MapTerrainType::regular, rand());
        mainData.chunks[x].pos = {x % 2, x / 2};
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
        if (cameraZoom > 6) cameraZoom = 6;

        double cameraZoomD = pow(2., cameraZoom);

        // Pan

        if (inputs[(uint8_t)Inputs::pan])
        {
            cameraPos += { ((double)mousePos.x / (double)windowSize.x - 0.5) * secondsPerFrame * 5 * cameraZoomD, ((double)mousePos.y / (double)windowSize.y - 0.5) * secondsPerFrame * 5 * cameraZoomD };
        }
        /*if (cameraPos.x < 0) cameraPos.x = 0;
        if (cameraPos.y < 0) cameraPos.y = 0;
        if (cameraPos.x > 256) cameraPos.x = 256;
        if (cameraPos.y > 256) cameraPos.y = 256;*/

        // Game logic

        /*for (uint8_t x = 0; x < 10; x++)
        {
            sf::Vector2<uint8_t> pos = { (uint8_t)rand() , (uint8_t)rand() };
            map[pos.x][pos.y].groundMaterial = (GroundMaterial)(rand() % 2);
            redrawTileQueue.push_back(pos);
        }*/

        if (inputs[(uint8_t)Inputs::f1]) generateMap(mainData, mainData.chunks[0], MapTerrainType::regular, rand());
        if (inputs[(uint8_t)Inputs::f2]) generateMap(mainData, mainData.chunks[0], MapTerrainType::swamp, rand());
        if (inputs[(uint8_t)Inputs::f3]) generateMap(mainData, mainData.chunks[0], MapTerrainType::flatGrass, rand());
        if (inputs[(uint8_t)Inputs::f4]) generateMap(mainData, mainData.chunks[0], MapTerrainType::flatWater, rand());

        // Render

        for (uint8_t x = 0; x < 4; x++)//4
        {
            renderChunk(mainData, mainData.chunks[x]);//x
        }

        /*for (; mainData.chunks[0].redrawQueue.size() > 0;)
        {
            sf::Vector2<uint8_t> pos = mainData.chunks[0].redrawQueue[mainData.chunks[0].redrawQueue.size() - 1];
            sf::Vector2<uint64_t> pos1 = {pos.x, pos.y};
            drawTile(mainData, mainData.chunks[0].tiles[pos.x + pos.y * 256], pos1, mainData.chunks[0]);
            mainData.chunks[0].redrawQueue.pop_back();
        }

        if (mainData.chunks[0].redrawAll)
        {
            mainData.triangles = {};
            mainData.freeTriangles = {};
            uint8_t x = 255;
            do
            {
                uint8_t y = 0;
                do
                {
                    drawTile(mainData, mainData.chunks[0].tiles[x + y * 256], { x, y }, mainData.chunks[0]);
                    y++;
                } while (y != 0);
                x--;
            } while (x != 255);
            mainData.chunks[0].redrawAll = 0;
        }*/

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

        /*if (1)//mainData.chunks[0].triangles.size() != 0
        {
            GLdouble* t1 = (GLdouble*)&mainData.chunks[0].triangles[0];
            glVertexPointer(3, GL_DOUBLE, 8 * sizeof(GLdouble), t1);
            glTexCoordPointer(2, GL_DOUBLE, 8 * sizeof(GLdouble), t1 + 4);
            glDrawArrays(GL_TRIANGLES, 0, mainData.chunks[0].triangles.size());
        }*/

        for (uint8_t x = 0; x < 4; x++)//4
        {
            uint8_t x1 = chunkLayout[1 - (x % 2)][x / 2];
            GLdouble* t1 = (GLdouble*)&mainData.chunks[x1].triangles[0];
            glVertexPointer(3, GL_DOUBLE, 8 * sizeof(GLdouble), t1);
            glTexCoordPointer(2, GL_DOUBLE, 8 * sizeof(GLdouble), t1 + 4);
            glDrawArrays(GL_TRIANGLES, 0, mainData.chunks[x1].triangles.size());
        }

        // GUI

        window.pushGLStates();

        //ScreenPos pos = ScreenPos({ -128, -128 }, ScreenPosHAlign::c, ScreenPosVAlign::c);
        //ScreenPos end = ScreenPos({ 128, 128 }, ScreenPosHAlign::c, ScreenPosVAlign::c);
        //renderRect(window, mainData, pos, end, sf::Color::Red);

        window.popGLStates();
        window.display();

        // Loop End

        for (uint8_t x = 0; x < (uint8_t)Inputs::size; x++) inputsLast[x] = inputs[x];
        lastSystemTime = systemTime;
    }

    delete[] mainData.chunks;
    return 0;
}
