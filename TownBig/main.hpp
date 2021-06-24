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
void generateMap(MainData& mainData, Map& map, MapTerrainType mapTerrainType, uint64_t seed);
const Version currentVersion = { 0, 2, 0, 0 };
void renderRect(sf::RenderWindow& window, MainData& windowData, ScreenPos pos, ScreenPos end, sf::Color color);
sf::Vector2f toVec2(ScreenPos screenPos, MainData& mainData);