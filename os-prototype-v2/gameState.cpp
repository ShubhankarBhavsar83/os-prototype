#include "GameState.h"
#include "LevelTile.h"
#include "Furniture.h"
#include "CoordinateSystem.h"
#include <iostream>
#include "LevelLoader.h" 
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

    if (playerPtr && !playerPtr->isActive()) {
        playerPtr = nullptr;
    }

}

void GameState::render(SDL_Renderer* renderer) {
    for (auto& layer : layers) {
        for (auto& entity : layer) {
            if (entity && entity->isActive()) {
                entity->render(renderer, mapViewport);
            }
        }
    }

   /* SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (auto& layer : layers) {
        for (auto& entity : layer) {
            if (entity && entity->isActive() && entity->isSolid()) {
                SDL_FRect box = entity->getBoundingBox();

                box.x -= mapViewport.x;
                box.y -= mapViewport.y;

                SDL_RenderRect(renderer, &box);
            }
        }*/
    //}

}

void GameState::checkCollisions() {
    if (!playerPtr) return;

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
    LevelLoader loader;

    // Load "abyss" or "corrode" or whatever your level name is
    // This will look for assets/levels/abyss_terrain.csv, etc.
    LevelData data = loader.loadLevel("abyss");

    // This function needs access to your ResourceManager.
    // Ensure your ResourceManager getter in GameState.h is public.
    if (resourceManager) {
        // Note: You might need to update populateGameState signature 
        // to take logicalWidth/Height if they aren't accessible globally or via GameState
        loader.populateGameState(data, *this, *resourceManager);
    }

    std::cout << "Level Loaded with Dimensions: " << data.width << "x" << data.height << std::endl;
}