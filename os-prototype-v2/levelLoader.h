#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Entity.h" 

struct LevelData {
    int width = 0;
    int height = 0;

    // Standard Layers
    std::vector<std::vector<int>> terrainLayer;
    std::vector<std::vector<int>> playerLayer;
    std::vector<std::vector<int>> furnitureLayer;
    std::vector<std::vector<int>> enemyLayer;
    std::vector<std::vector<int>> npcLayer;

    // NEW: Split Portal Layers (Must match what your Level Editor exports)
    std::vector<std::vector<int>> portalLayer;
    //std::vector<std::vector<int>> portalFgLayer;
};

class LevelLoader {
public:
    LevelLoader();

    LevelData loadLevel(const std::string& levelName);
    std::vector<std::vector<int>> parseCSVGrid(const std::string& filepath);

    void populateGameState(const LevelData& data, class GameState& gs, class ResourceManager& res);
};

