#include "GameState.h"
#include "LevelTile.h"
#include "Furniture.h"
#include "CoordinateSystem.h"
#include <iostream>

const int MAP_ROWS = 25;
const int MAP_COLS = 25;
const int TILE_SIZE = 32;

GameState::GameState(SDL_Renderer* renderer, int viewportWidth, int viewportHeight)
    : mode(GameStateMode::PLAYING), playerPtr(nullptr),
    logicalWidth(viewportWidth), logicalHeight(viewportHeight) {

    mapViewport = {
        .x = 0,
        .y = 0,
        .w = static_cast<float>(viewportWidth),
        .h = static_cast<float>(viewportHeight)
    };

    resourceManager = std::make_unique<ResourceManager>(renderer);
    resourceManager->loadAllAssets();
}

void GameState::update(float deltaTime) {
    if (mode != GameStateMode::PLAYING) return;

    // Update all entities
    for (auto& layer : layers) {
        for (auto& entity : layer) {
            if (entity && entity->isActive()) {
                entity->update(deltaTime, *this);
            }
        }
    }

    checkCollisions();
    updateCamera();
    cleanup();
}

void GameState::render(SDL_Renderer* renderer) {
    for (auto& layer : layers) {
        for (auto& entity : layer) {
            if (entity && entity->isActive()) {
                entity->render(renderer, mapViewport);
            }
        }
    }
}

void GameState::checkCollisions() {
    //if (!playerPtr) return;

    // Check player against all solid entities
    for (auto& layer : layers) {
        for (auto& entity : layer) {
            if (entity && entity->isActive() && entity.get() != playerPtr && entity->isSolid()) {
                playerPtr->handleCollision(entity.get());
            }
        }
    }
}

void GameState::cleanup() {
    for (auto& layer : layers) {
        layer.erase(
            std::remove_if(layer.begin(), layer.end(),
                [](const std::unique_ptr<Entity>& e) { return !e->isActive(); }),
            layer.end()
        );
    }
}

void GameState::addEntity(std::unique_ptr<Entity> entity, size_t layer) {
    if (layer >= layers.size()) return;

    entity->setCurrentLayer(layer);

    if (entity->getType() == EntityType::PLAYER) {
        playerPtr = static_cast<Player*>(entity.get());
    }

    layers[layer].push_back(std::move(entity));
}

void GameState::moveEntityToLayer(Entity* entity, size_t newLayer) {
    if (!entity || newLayer >= layers.size()) return;

    size_t oldLayer = entity->getCurrentLayer();
    if (oldLayer == newLayer) return;

    auto& oldVec = layers[oldLayer];
    auto it = std::find_if(oldVec.begin(), oldVec.end(),
        [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; });

    if (it != oldVec.end()) {
        std::unique_ptr<Entity> moved = std::move(*it);
        oldVec.erase(it);
        moved->setCurrentLayer(newLayer);
        layers[newLayer].push_back(std::move(moved));
    }
}

void GameState::updateCamera() {
    if (!playerPtr) return;

    glm::vec2 playerPos = playerPtr->getPosition();
    mapViewport.x = playerPos.x - mapViewport.w / 2.0f;
    mapViewport.y = playerPos.y - mapViewport.h / 2.0f;
}

void GameState::loadTestLevel() {
    short terrain_map[MAP_ROWS][MAP_COLS] = {
        {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1},
        {1,1,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,2},
        {1,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,2,2},
        {1,1,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2},
        {1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2},
        {1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,2,2},
        {1,1,1,1,1,2,2,1,2,2,1,1,1,1,1,2,2,1,2,2,1,1,1,1,2},
        {1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2},
        {1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2},
        {1,1,1,2,2,2,1,1,1,2,1,1,1,2,2,2,1,1,1,2,1,1,2,2,2},
        {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1},
        {1,1,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,2},
        {1,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,2,2},
        {1,1,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2},
        {1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2},
        {1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,2,2},
        {1,1,1,1,1,2,2,1,2,2,1,1,1,1,1,2,2,1,2,2,1,1,1,1,2},
        {1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2},
        {1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2},
        {1,1,1,2,2,2,1,1,1,2,1,1,1,2,2,2,1,1,1,2,1,1,2,2,2},
        {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1},
        {1,1,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,2},
        {1,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,2,2},
        {1,1,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2},
        {1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2}
    };

    short furniture_map[MAP_ROWS][MAP_COLS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };

    // Create tiles
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            glm::vec2 isoPos = CoordinateSystem::orthoToIso(c, r, TILE_SIZE, logicalWidth, logicalHeight);

            // Terrain
            if (terrain_map[r][c] == 1) {
                auto tile = std::make_unique<LevelTile>(TileType::FLOOR_DIRT);
                tile->setPosition(isoPos);
                tile->texture = resourceManager->getTexture("tile_dirt");
                addEntity(std::move(tile), LAYER_IDX_LEVEL);
            }
            else if (terrain_map[r][c] == 2) {
                auto tile = std::make_unique<LevelTile>(TileType::FLOOR_GRASS);
                tile->setPosition(isoPos);
                tile->texture = resourceManager->getTexture("tile_grass");
                addEntity(std::move(tile), LAYER_IDX_LEVEL);
            }

            // Furniture
            if (furniture_map[r][c] == 5) {
                auto furn = std::make_unique<Furniture>();
                furn->setPosition(isoPos);
                furn->texture = resourceManager->getTexture("tile_pillar");
                int test = 300 + (r * MAP_COLS + c);
                furn->setId(test);
                addEntity(std::move(furn), LAYER_IDX_FURNITURE_BACKGROUND);
            }
        }
    }

    // Create player at position (0, 0)
    glm::vec2 playerPos = CoordinateSystem::orthoToIso(0, 0, TILE_SIZE, logicalWidth, logicalHeight);
    auto player = std::make_unique<Player>();
    player->setPosition(playerPos);
    player->texture = resourceManager->getTexture("player_idle");
    player->animations = resourceManager->getAnimationSet("player");
    player->playAnimation(1); // IDLE animation
    addEntity(std::move(player), LAYER_IDX_CHARACTERS);
}