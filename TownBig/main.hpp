#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/OpenGL.hpp"
#include "types.hpp"

void initWindow(sf::RenderWindow& window, MainData& windowData, bool firstTime);
PosSize appendTris(MainData& mainData, std::vector<TriPoint>& newTris);
void drawTile(MainData& mainData, Tile& tile, sf::Vector2<uint8_t> pos, Map& map);
void deleteTris(MainData& mainData, PosSize posSize);
uint64_t getSystemTime();
const char* textureNames[];