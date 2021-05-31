#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/OpenGL.hpp"
#include "types.hpp"

void initWindow(sf::RenderWindow& window, MainData& windowData, bool firstTime);
PosSize appendTris(MainData& mainData, std::vector<TriPoint>& newTris);
void drawTile(MainData& mainData, Tile& tile, sf::Vector2<uint8_t>);
void deleteTris(MainData& mainData, PosSize posSize);