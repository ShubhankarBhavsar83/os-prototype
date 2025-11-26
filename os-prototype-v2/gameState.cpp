#include "GameState.h"
#include "LevelTile.h"
#include "Furniture.h"
#include "CoordinateSystem.h"
#include <iostream>
#include "LevelLoader.h"
#include "FriendlyNpc.h"
#include "EnemyNpc.h"

const int MAP_ROWS = 25;
const int MAP_COLS = 25;
const int TILE_SIZE = 32;

GameState::GameState(SDL_Renderer* renderer, int viewportWidth, int viewportHeight)
    : mode(GameStateMode::PLAYING), playerPtr(nullptr),
    logicalWidth(viewportWidth), logicalHeight(viewportHeight) {

    mapViewport = {
        .x = 0, .y = 0,
        .w = static_cast<float>(viewportWidth),
        .h = static_cast<float>(viewportHeight)
    };

    resourceManager = std::make_unique<ResourceManager>(renderer);
    resourceManager->loadAllAssets();
}

void GameState::update(float deltaTime) {
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
    if (!playerPtr) return;
    for (auto& layer : layers) {
        for (auto& entity : layer) {
            if (entity && entity->isActive() && entity.get() != playerPtr) {
                if (entity->isSolid()) playerPtr->handleCollision(entity.get());
                if (entity->getType() == EntityType::PORTAL) playerPtr->handleCollision(entity.get());
            }
        }
    }
}

void GameState::cleanup() {
    for (auto& layer : layers) {
        layer.erase(std::remove_if(layer.begin(), layer.end(),
            [](const std::unique_ptr<Entity>& e) { return !e->isActive(); }),
            layer.end());
    }
}

void GameState::addEntity(std::unique_ptr<Entity> entity, size_t layer) {
    if (layer >= layers.size()) return;
    entity->setCurrentLayer(layer);
    if (entity->getType() == EntityType::PLAYER) playerPtr = static_cast<Player*>(entity.get());
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

void GameState::loadLevel() {
    LevelLoader loader;
    LevelData data = loader.loadLevel("abyss");
    if (resourceManager) {
        loader.populateGameState(data, *this, *resourceManager);
    }
}

void GameState::loadLevel(const std::string& levelName) {
    LevelLoader loader;
    LevelData data = loader.loadLevel(levelName);
    if (resourceManager) {
        loader.populateGameState(data, *this, *resourceManager);
    }
}

// Find nearest friendly NPC
FriendlyNPC* GameState::findNearestFriendlyNPC(glm::vec2 position, float range) {
    FriendlyNPC* nearest = nullptr;
    float minDist = range;

    if (LAYER_IDX_CHARACTERS < layers.size()) {
        for (auto& entity : layers[LAYER_IDX_CHARACTERS]) {
            if (entity->getType() == EntityType::FRIENDLY_NPC && entity->isActive()) {
                float dist = glm::distance(position, entity->getPosition());
                if (dist < minDist) {
                    minDist = dist;
                    nearest = static_cast<FriendlyNPC*>(entity.get());
                }
            }
        }
    }
    return nearest;
}

// Get all enemies in range
std::vector<Entity*> GameState::getEnemiesInRange(glm::vec2 position, float range) {
    std::vector<Entity*> enemies;

    if (LAYER_IDX_CHARACTERS < layers.size()) {
        for (auto& entity : layers[LAYER_IDX_CHARACTERS]) {
            if (entity->getType() == EntityType::ENEMY && entity->isActive()) {
                float dist = glm::distance(position, entity->getPosition());
                if (dist <= range) {
                    enemies.push_back(entity.get());
                }
            }
        }
    }
    return enemies;
}

// Get all entities in range on a specific layer
std::vector<Entity*> GameState::getEntitiesInRange(glm::vec2 position, float range, size_t layer) {
    std::vector<Entity*> entities;

    if (layer < layers.size()) {
        for (auto& entity : layers[layer]) {
            if (entity->isActive()) {
                float dist = glm::distance(position, entity->getPosition());
                if (dist <= range) {
                    entities.push_back(entity.get());
                }
            }
        }
    }
    return entities;
}
