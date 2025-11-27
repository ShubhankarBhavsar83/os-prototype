#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Entity.h" 
#include "Collider.h"  // NEW: Include Collider

struct LevelData {
    int width = 0;
    int height = 0;

    // Standard Layers
    std::vector<std::vector<int>> terrainLayer;
    std::vector<std::vector<int>> playerLayer;
    std::vector<std::vector<int>> furnitureLayer;
    std::vector<std::vector<int>> enemyLayer;
    std::vector<std::vector<int>> npcLayer;

    // Portal Layer
    std::vector<std::vector<int>> portalLayer;

    // Scale Layers
    std::vector<std::vector<float>> terrainScale;
    std::vector<std::vector<float>> playerScale;
    std::vector<std::vector<float>> furnitureScale;
    std::vector<std::vector<float>> enemyScale;
    std::vector<std::vector<float>> npcScale;
    std::vector<std::vector<float>> portalScale;
};

class LevelLoader {
public:
    LevelLoader();

    LevelData loadLevel(const std::string& levelName);
    std::vector<std::vector<int>> parseCSVGrid(const std::string& filepath);
    std::vector<std::vector<float>> parseCSVGridFloat(const std::string& filepath);

    void populateGameState(const LevelData& data, class GameState& gs, class ResourceManager& res);

    // NEW: Collision data loading functions
    void loadTileCollisionData();
    Collider getColliderForTileID(int tileID, float baseWidth, float baseHeight, float scale);
};