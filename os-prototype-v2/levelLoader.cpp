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
#include <map>

// Global collision data storage
struct TileColliderData {
    bool isSolid = false;
    int offsetLeft = 0;
    int offsetRight = 0;
    int offsetTop = 0;
    int offsetBottom = 0;
};

static std::map<int, TileColliderData> gTileColliders;

LevelLoader::LevelLoader() {}

std::vector<std::vector<float>> LevelLoader::parseCSVGridFloat(const std::string& filepath) {
    std::vector<std::vector<float>> grid;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cout << "[LevelLoader] Note: Scale file not found: " << filepath << " (Using default 1.0)" << std::endl;
        return grid;
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        if (firstLine) {
            if (line.size() >= 3 && (unsigned char)line[0] == 0xEF) line = line.substr(3);
            firstLine = false;
        }
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();

        std::vector<float> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            try {
                if (!cell.empty()) {
                    cell.erase(std::remove(cell.begin(), cell.end(), ' '), cell.end());
                    if (!cell.empty()) row.push_back(std::stof(cell));
                    else row.push_back(1.0f);
                }
                else row.push_back(1.0f);
            }
            catch (...) { row.push_back(1.0f); }
        }
        if (!row.empty()) grid.push_back(row);
    }
    return grid;
}

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
    data.portalLayer = parseCSVGrid(basePath + "_portal.csv");

    // Load Scale layers
    data.terrainScale = parseCSVGridFloat(basePath + "_terrain_scale.csv");
    data.playerScale = parseCSVGridFloat(basePath + "_player_scale.csv");
    data.furnitureScale = parseCSVGridFloat(basePath + "_furniture_scale.csv");
    data.enemyScale = parseCSVGridFloat(basePath + "_enemy_scale.csv");
    data.npcScale = parseCSVGridFloat(basePath + "_npcs_scale.csv");
    data.portalScale = parseCSVGridFloat(basePath + "_portal_scale.csv");

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
        std::cout << "[LevelLoader] Note: File not found: " << filepath << std::endl;
        return grid;
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        if (firstLine) {
            if (line.size() >= 3 && (unsigned char)line[0] == 0xEF) line = line.substr(3);
            firstLine = false;
        }
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

std::string formatTileID(int id) {
    std::stringstream ss;
    ss << std::setw(3) << std::setfill('0') << id;
    return ss.str();
}

float getScaleAt(const std::vector<std::vector<float>>& scaleGrid, int r, int c) {
    if (r >= 0 && r < (int)scaleGrid.size() &&
        c >= 0 && c < (int)scaleGrid[r].size()) {
        return scaleGrid[r][c];
    }
    return 1.0f;
}

// NEW: Load collision data from CSV
void LevelLoader::loadTileCollisionData() {
    std::string colliderFile = "assets/levels/tile_colliders.csv";
    std::ifstream file(colliderFile);

    if (!file.is_open()) {
        std::cout << "[LevelLoader] Warning: tile_colliders.csv not found. No collision data loaded." << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Remove trailing whitespace
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 6) {
            try {
                int tileID = std::stoi(tokens[0]) - 1; // Convert from 1-based to 0-based
                TileColliderData collider;
                collider.isSolid = (std::stoi(tokens[1]) == 1);
                collider.offsetLeft = std::stoi(tokens[2]);
                collider.offsetRight = std::stoi(tokens[3]);
                collider.offsetTop = std::stoi(tokens[4]);
                collider.offsetBottom = std::stoi(tokens[5]);

                gTileColliders[tileID] = collider;
            }
            catch (...) {
                std::cerr << "[LevelLoader] Error parsing collision data for line: " << line << std::endl;
            }
        }
    }

    file.close();
    std::cout << "[LevelLoader] Loaded " << gTileColliders.size() << " tile collision definitions." << std::endl;
}

// NEW: Get collider for a tile ID
Collider LevelLoader::getColliderForTileID(int tileID, float baseWidth, float baseHeight, float scale) {
    auto it = gTileColliders.find(tileID);

    if (it != gTileColliders.end() && it->second.isSolid) {
        const TileColliderData& data = it->second;
        return Collider(
            data.offsetLeft * scale,
            data.offsetRight * scale,
            data.offsetTop * scale,
            data.offsetBottom * scale,
            true  // isSolid = true
        );
    }

    // Return non-solid collider for tiles without collision data
    return Collider(0, 0, 0, 0, false);
}

void LevelLoader::populateGameState(const LevelData& data, GameState& gs, ResourceManager& res) {
    int logicalW = 640;
    int logicalH = 320;
    int TILE_SIZE = 32;

    // NEW: Load collision data before populating
    loadTileCollisionData();

    std::cout << "[LevelLoader] Populating Game State..." << std::endl;

    // 1. TERRAIN (with scale AND collision)
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
            tile->scale = getScaleAt(data.terrainScale, r, c);

            // NEW: Apply collision data from CSV
            tile->collider = getColliderForTileID(id, 32.0f, 32.0f, tile->scale);

            gs.addEntity(std::move(tile), LAYER_IDX_LEVEL);
        }
    }

    // 2. FURNITURE (with scale AND collision)
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
                furn->scale = getScaleAt(data.furnitureScale, r, c);

                // NEW: Apply collision data from CSV
                furn->collider = getColliderForTileID(id, 32.0f, 32.0f, furn->scale);

                gs.addEntity(std::move(furn), LAYER_IDX_FURNITURE_BACKGROUND);
            }
        }
    }

    // 3. PLAYER (scale doesn't typically apply, but keeping structure)
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

    // 4. ENEMY (with scale)
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
                    enemy->scale = getScaleAt(data.enemyScale, r, c);
                    gs.addEntity(std::move(enemy), LAYER_IDX_CHARACTERS);
                }
                catch (...) {
                    std::cerr << "[LevelLoader] Error spawning Enemy at " << c << "," << r << std::endl;
                }
            }
        }
    }

    // 5. NPC (with scale)
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
                    npc->scale = getScaleAt(data.npcScale, r, c);
                    gs.addEntity(std::move(npc), LAYER_IDX_CHARACTERS);
                }
                catch (...) {
                    std::cerr << "[LevelLoader] Error spawning NPC at " << c << "," << r << std::endl;
                }
            }
        }
    }

    // 6. PORTAL (with scale AND collision)
    if (!data.portalLayer.empty()) {
        for (int r = 0; r < data.height; ++r) {
            for (int c = 0; c < data.width; ++c) {
                if (r >= (int)data.portalLayer.size() || c >= (int)data.portalLayer[r].size()) continue;
                int id = data.portalLayer[r][c];
                if (id <= 0) continue;

                std::string texKey = "tile_" + std::to_string(id);
                if (!res.getTexture(texKey)) {
                    res.loadTexture(texKey, "assets/map_assets/tile_" + formatTileID(id) + ".png");
                }

                auto portal = std::make_unique<Portal>();
                portal->texture = res.getTexture(texKey);
                portal->setPosition(CoordinateSystem::orthoToIso(c, r, TILE_SIZE, logicalW, logicalH));
                portal->scale = getScaleAt(data.portalScale, r, c);

                // NEW: Apply collision data from CSV
                portal->collider = getColliderForTileID(id, 32.0f, 32.0f, portal->scale);

                gs.addEntity(std::move(portal), LAYER_IDX_PORTAL_BACKGROUND);
            }
        }
    }

    std::cout << "[LevelLoader] Level population complete." << std::endl;
}