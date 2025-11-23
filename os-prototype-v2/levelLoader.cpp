#include "LevelLoader.h"
#include "GameState.h"
#include "LevelTile.h"
#include "Player.h"
#include "Furniture.h"
#include "EnemyNpc.h"     
#include "FriendlyNpc.h"  
#include "CoordinateSystem.h"
#include "Portal.h"        
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

LevelLoader::LevelLoader() {}

LevelData LevelLoader::loadLevel(const std::string& levelName) {
    LevelData data;
    std::string basePath = "assets/levels/" + levelName;

    std::cout << "[LevelLoader] Loading Level: " << levelName << std::endl;

    // Load all layers
    data.terrainLayer = parseCSVGrid(basePath + "_terrain.csv");
    data.playerLayer = parseCSVGrid(basePath + "_player.csv");
    data.furnitureLayer = parseCSVGrid(basePath + "_furniture.csv");
    data.enemyLayer = parseCSVGrid(basePath + "_enemy.csv");
    data.npcLayer = parseCSVGrid(basePath + "_npcs.csv");
    data.portalLayer = parseCSVGrid(basePath + "_portal.csv"); // Loading single portal layer

    // Set dimensions from terrain layer
    if (!data.terrainLayer.empty()) {
        data.height = (int)data.terrainLayer.size();
        data.width = (int)data.terrainLayer[0].size();
    }
    else {
        std::cerr << "[LevelLoader] ERROR: Terrain layer is empty or failed to load!" << std::endl;
    }

    return data;
}

std::vector<std::vector<int>> LevelLoader::parseCSVGrid(const std::string& filepath) {
    std::vector<std::vector<int>> grid;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        // Warn but don't crash (Portals might be optional)
        std::cout << "[LevelLoader] Note: File not found: " << filepath << std::endl;
        return grid;
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        // BOM FIX
        if (firstLine) {
            if (line.size() >= 3 && (unsigned char)line[0] == 0xEF) line = line.substr(3);
            firstLine = false;
        }
        // Cleanup
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();

        std::vector<int> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            try {
                if (!cell.empty()) {
                    cell.erase(std::remove(cell.begin(), cell.end(), ' '), cell.end());
                    if (!cell.empty()) row.push_back(std::stoi(cell));
                    else row.push_back(0);
                }
                else row.push_back(0);
            }
            catch (...) { row.push_back(0); }
        }
        if (!row.empty()) grid.push_back(row);
    }
    return grid;
}

// Helper
std::string formatTileID(int id) {
    std::stringstream ss;
    ss << std::setw(3) << std::setfill('0') << id;
    return ss.str();
}

void LevelLoader::populateGameState(const LevelData& data, GameState& gs, ResourceManager& res) {
    int logicalW = 640;
    int logicalH = 320;
    int TILE_SIZE = 32;

    std::cout << "[LevelLoader] Populating Game State..." << std::endl;

    // 1. TERRAIN
    for (int r = 0; r < data.height; ++r) {
        for (int c = 0; c < data.width; ++c) {
            if (r >= (int)data.terrainLayer.size() || c >= (int)data.terrainLayer[r].size()) continue;
            int id = data.terrainLayer[r][c];
            if (id <= 0) continue;

            std::string texKey = "tile_" + std::to_string(id);
            if (!res.getTexture(texKey)) {
                res.loadTexture(texKey, "assets/map_assets/tile_" + formatTileID(id) + ".png");
            }

            auto tile = std::make_unique<LevelTile>(TileType::FLOOR_DIRT);
            tile->texture = res.getTexture(texKey);
            tile->setPosition(CoordinateSystem::orthoToIso(c, r, TILE_SIZE, logicalW, logicalH));
            gs.addEntity(std::move(tile), LAYER_IDX_LEVEL);
        }
    }

    // 2. FURNITURE
    if (!data.furnitureLayer.empty()) {
        for (int r = 0; r < data.height; ++r) {
            for (int c = 0; c < data.width; ++c) {
                if (r >= (int)data.furnitureLayer.size() || c >= (int)data.furnitureLayer[r].size()) continue;
                int id = data.furnitureLayer[r][c];
                if (id <= 0) continue;

                std::string texKey = "tile_" + std::to_string(id);
                if (!res.getTexture(texKey)) res.loadTexture(texKey, "assets/map_assets/tile_" + formatTileID(id) + ".png");

                auto furn = std::make_unique<Furniture>();
                furn->texture = res.getTexture(texKey);
                furn->setPosition(CoordinateSystem::orthoToIso(c, r, TILE_SIZE, logicalW, logicalH));
                gs.addEntity(std::move(furn), LAYER_IDX_FURNITURE_BACKGROUND);
            }
        }
    }

    // 3. PLAYER
    if (!data.playerLayer.empty()) {
        for (int r = 0; r < data.height; ++r) {
            for (int c = 0; c < data.width; ++c) {
                if (r >= (int)data.playerLayer.size() || c >= (int)data.playerLayer[r].size()) continue;
                int id = data.playerLayer[r][c];
                if (id == 3 || id == 54) {
                    glm::vec2 pos = CoordinateSystem::orthoToIso(c, r, TILE_SIZE, logicalW, logicalH);
                    auto player = std::make_unique<Player>();
                    player->setPosition(pos);
                    player->texture = res.getTexture("player_idle");
                    player->animations = res.getAnimationSet("player");
                    player->playAnimation(1);
                    gs.addEntity(std::move(player), LAYER_IDX_CHARACTERS);
                }
            }
        }
    }

    // 4. ENEMY
    if (!data.enemyLayer.empty()) {
        for (int r = 0; r < data.height; ++r) {
            for (int c = 0; c < data.width; ++c) {
                if (r >= (int)data.enemyLayer.size() || c >= (int)data.enemyLayer[r].size()) continue;
                int id = data.enemyLayer[r][c];
                if (id <= 0) continue;

                try {
                    glm::vec2 pos = CoordinateSystem::orthoToIso(c, r, TILE_SIZE, logicalW, logicalH);
                    auto enemy = std::make_unique<EnemyNPC>(EnemyAIType::MELEE);
                    enemy->setPosition(pos);
                    gs.addEntity(std::move(enemy), LAYER_IDX_CHARACTERS);
                }
                catch (...) {
                    std::cerr << "[LevelLoader] Error spawning Enemy at " << c << "," << r << std::endl;
                }
            }
        }
    }

    // 5. NPC
    if (!data.npcLayer.empty()) {
        for (int r = 0; r < data.height; ++r) {
            for (int c = 0; c < data.width; ++c) {
                if (r >= (int)data.npcLayer.size() || c >= (int)data.npcLayer[r].size()) continue;
                int id = data.npcLayer[r][c];
                if (id <= 0) continue;

                try {
                    glm::vec2 pos = CoordinateSystem::orthoToIso(c, r, TILE_SIZE, logicalW, logicalH);
                    auto npc = std::make_unique<FriendlyNPC>();
                    npc->setPosition(pos);
                    gs.addEntity(std::move(npc), LAYER_IDX_CHARACTERS);
                }
                catch (...) {
                    std::cerr << "[LevelLoader] Error spawning NPC at " << c << "," << r << std::endl;
                }
            }
        }
    }

    // 6. PORTAL (Fixed Texture Loading)
    if (!data.portalLayer.empty()) {
        for (int r = 0; r < data.height; ++r) {
            for (int c = 0; c < data.width; ++c) {
                if (r >= (int)data.portalLayer.size() || c >= (int)data.portalLayer[r].size()) continue;
                int id = data.portalLayer[r][c];
                if (id <= 0) continue;

                // --- START FIX: Load Texture ---
                std::string texKey = "tile_" + std::to_string(id);
                if (!res.getTexture(texKey)) {
                    // Loads the image, e.g., "assets/map_assets/tile_150.png"
                    res.loadTexture(texKey, "assets/map_assets/tile_" + formatTileID(id) + ".png");
                }
                // -------------------------------

                auto portal = std::make_unique<Portal>();
                portal->texture = res.getTexture(texKey); // Assign the texture!
                portal->setPosition(CoordinateSystem::orthoToIso(c, r, TILE_SIZE, logicalW, logicalH));

                // Using BACKGROUND layer as per your GameState setup
                gs.addEntity(std::move(portal), LAYER_IDX_PORTAL_BACKGROUND);
            }
        }
    }

    std::cout << "[LevelLoader] Level population complete." << std::endl;
}