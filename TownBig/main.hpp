#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/OpenGL.hpp"
#include "types.hpp"

void initWindow(sf::RenderWindow& window, MainData& windowData, bool firstTime);
//PosSize appendTris(MainData& mainData, std::vector<TriPoint>& newTris);
void drawTile(MainData& mainData, Tile& tile, sf::Vector2<uint64_t> pos, Chunk& map);
//void deleteTris(MainData& mainData, PosSize posSize);
uint64_t getSystemTime();
const char* textureNames[];
void generateMap(MainData& mainData, Chunk& map, MapTerrainType mapTerrainType, uint64_t seed);
const Version currentVersion = { 1, 2, 0, 0 };
void renderRect(sf::RenderWindow& window, MainData& windowData, ScreenPos pos, ScreenPos end, sf::Color color);
sf::Vector2f toVec2(ScreenPos screenPos, MainData& mainData);
void renderChunk(MainData& mainData, Chunk& chunk);