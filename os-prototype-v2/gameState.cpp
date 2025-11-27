#include "GameState.h"
#include "LevelTile.h"
#include "Furniture.h"
#include "CoordinateSystem.h"
#include "LevelLoader.h"
#include "FriendlyNpc.h"
#include "EnemyNpc.h"
#include "Projectile.h"
#include <iostream>
#include <algorithm>

// Define constants (remove extern declarations)
const int MAP_ROWS = 25;
const int MAP_COLS = 25;
const int TILE_SIZE = 32;

GameState::GameState(SDL_Renderer* renderer, int viewportWidth, int viewportHeight)
    : mode(GameStateMode::PLAYING), playerPtr(nullptr),
    logicalWidth(viewportWidth), logicalHeight(viewportHeight),
    currentLevel(""), levelTransitionPending(false),
    collisionCheckRadius(300.0f) {

    mapViewport = {
        .x = 0,
        .y = 0,
        .w = static_cast<float>(viewportWidth),
        .h = static_cast<float>(viewportHeight)
    };

    resourceManager = std::make_unique<ResourceManager>(renderer);
    resourceManager->loadAllAssets();

    uiManager = std::make_unique<UIManager>(renderer);

    // Setup UI ability icons (RESTORED from old version)
    uiManager->addAbilityIcon("Dash", "assets/ui/dash_icon.png", 5.0f);
    uiManager->addAbilityIcon("Melee", "assets/ui/melee_icon.png", 0.6f);
    uiManager->addAbilityIcon("Fireball", "assets/ui/fireball_icon.png", 2.0f);
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
    checkMeleeAttacks();      // RESTORED from old version
    checkEnemyAttacks();       // RESTORED from old version
    updateCamera();
    cleanup();

    // Update UI (RESTORED from old version)
    uiManager->update(deltaTime, *this);

    if (playerPtr && !playerPtr->isActive()) {
        playerPtr = nullptr;
    }

    // Handle level transitions (RESTORED from old version)
    if (levelTransitionPending && !nextLevel.empty()) {
        loadLevel(nextLevel);
        levelTransitionPending = false;
        nextLevel = "";
    }
}

void GameState::render(SDL_Renderer* renderer) {
    // Render all entity layers
    for (auto& layer : layers) {
        for (auto& entity : layer) {
            if (entity && entity->isActive()) {
                entity->render(renderer, mapViewport);
            }
        }
    }

    // ========================================================================
    // DEBUG: Render Collision Boxes
    // ========================================================================
    //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color for collision
    //for (auto& layer : layers) {
    //    for (auto& entity : layer) {
    //        // Draw box if entity is active and has solid collision
    //        if (entity && entity->isActive() && entity->isSolid()) {
    //            SDL_FRect box = entity->getBoundingBox();

    //            // Adjust for camera viewport to get screen coordinates
    //            SDL_FRect screenBox = {
    //                box.x - mapViewport.x,
    //                box.y - mapViewport.y,
    //                box.w,
    //                box.h
    //            };

    //            SDL_RenderRect(renderer, &screenBox);
    //        }
    //    }
    //}
    // ========================================================================

    // Render UI on top (RESTORED from old version)
    uiManager->render(*this);
}

void GameState::checkCollisions() {
    if (!playerPtr) return;

    checkPlayerCollisions();
    checkProjectileCollisions();  // RESTORED from old version
}

void GameState::checkPlayerCollisions() {
    glm::vec2 playerPos = playerPtr->getPosition();

    for (auto& layer : layers) {
        for (auto& entity : layer) {
            if (entity && entity->isActive() &&
                entity.get() != playerPtr && entity->isSolid()) {

                float dist = glm::distance(playerPos, entity->getPosition());
                if (dist < collisionCheckRadius) {
                    playerPtr->handleCollision(entity.get());
                }
            }
        }
    }
}

void GameState::checkProjectileCollisions() {
    auto& projectileLayer = layers[LAYER_IDX_PROJECTILES];
    auto& characterLayer = layers[LAYER_IDX_CHARACTERS];

    for (auto& proj : projectileLayer) {
        if (!proj || !proj->isActive()) continue;

        Projectile* projectile = static_cast<Projectile*>(proj.get());
        SDL_FRect projBox = projectile->getBoundingBox();

        // Check against characters
        for (auto& entity : characterLayer) {
            if (!entity || !entity->isActive()) continue;
            if (entity.get() == proj.get()) continue;

            SDL_FRect entityBox = entity->getBoundingBox();
            SDL_FRect intersection;

            if (SDL_GetRectIntersectionFloat(&projBox, &entityBox, &intersection)) {
                projectile->handleCollision(entity.get());
                break;
            }
        }
    }
}

// ============================================================================
// COMBAT COLLISION DETECTION (RESTORED from old version)
// ============================================================================

void GameState::checkMeleeAttacks() {
    if (!playerPtr || !playerPtr->isCurrentlyAttacking()) return;

    SDL_FRect meleeBox = playerPtr->getMeleeHitbox();

    // Check melee hitbox against enemies
    for (auto& entity : layers[LAYER_IDX_CHARACTERS]) {
        if (!entity || !entity->isActive()) continue;
        if (entity->getType() != EntityType::ENEMY) continue;

        SDL_FRect enemyBox = entity->getBoundingBox();
        SDL_FRect intersection;

        if (SDL_GetRectIntersectionFloat(&meleeBox, &enemyBox, &intersection)) {
            EnemyNPC* enemy = static_cast<EnemyNPC*>(entity.get());

            // TUNING: Player melee damage
            float playerMeleeDamage = 25.0f; // <-- Adjust this value

            enemy->takeDamage(playerMeleeDamage);
            std::cout << "[Combat] Player melee hit enemy for " << playerMeleeDamage << " damage!" << std::endl;
        }
    }
}

// NEW: Check if enemies are hitting player with their attacks
void GameState::checkEnemyAttacks() {
    if (!playerPtr) return;

    SDL_FRect playerBox = playerPtr->getBoundingBox();

    for (auto& entity : layers[LAYER_IDX_CHARACTERS]) {
        if (!entity || !entity->isActive()) continue;
        if (entity->getType() != EntityType::ENEMY) continue;

        EnemyNPC* enemy = static_cast<EnemyNPC*>(entity.get());

        // Only check if enemy is currently attacking
        if (enemy->isCurrentlyAttacking()) {
            SDL_FRect enemyAttackBox = enemy->getAttackHitbox();
            SDL_FRect intersection;

            if (SDL_GetRectIntersectionFloat(&playerBox, &enemyAttackBox, &intersection)) {
                // Get enemy's attack damage (already tuned per variant)
                float enemyDamage = enemy->getAttackDamage();

                playerPtr->takeDamage(enemyDamage);
                std::cout << "[Combat] Enemy hit player for " << enemyDamage << " damage!" << std::endl;
            }
        }
    }
}

// ============================================================================

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

std::vector<Entity*> GameState::getEntitiesInLayer(size_t layer) {
    std::vector<Entity*> result;
    if (layer >= layers.size()) return result;

    for (auto& entity : layers[layer]) {
        if (entity && entity->isActive()) {
            result.push_back(entity.get());
        }
    }
    return result;
}

std::vector<Entity*> GameState::getEntitiesInRange(const glm::vec2& pos, float range, size_t layer) {
    std::vector<Entity*> result;
    if (layer >= layers.size()) return result;

    for (auto& entity : layers[layer]) {
        if (entity && entity->isActive()) {
            float dist = glm::distance(pos, entity->getPosition());
            if (dist <= range) {
                result.push_back(entity.get());
            }
        }
    }
    return result;
}

std::vector<Entity*> GameState::getEnemiesInRange(const glm::vec2& pos, float range) {
    std::vector<Entity*> enemies;

    for (auto& entity : layers[LAYER_IDX_CHARACTERS]) {
        if (entity && entity->isActive() && entity->getType() == EntityType::ENEMY) {
            EnemyNPC* enemy = static_cast<EnemyNPC*>(entity.get());
            if (!enemy->isDead()) {
                float dist = glm::distance(pos, entity->getPosition());
                if (dist <= range) {
                    enemies.push_back(entity.get());
                }
            }
        }
    }

    // Sort by distance
    std::sort(enemies.begin(), enemies.end(),
        [&pos](Entity* a, Entity* b) {
            return glm::distance(pos, a->getPosition()) <
                glm::distance(pos, b->getPosition());
        });

    return enemies;
}

void GameState::updateCamera() {
    if (!playerPtr) return;

    glm::vec2 playerPos = playerPtr->getPosition();
    mapViewport.x = playerPos.x - mapViewport.w / 2.0f;
    mapViewport.y = playerPos.y - mapViewport.h / 2.0f;
}

void GameState::clearLevel() {
    for (auto& layer : layers) {
        layer.clear();
    }
    playerPtr = nullptr;
}

void GameState::loadLevel() {
    LevelLoader loader;
    LevelData data = loader.loadLevel("abyss");
    if (resourceManager) {
        loader.populateGameState(data, *this, *resourceManager);
    }
}

void GameState::loadLevel(const std::string& levelName) {
    clearLevel();

    LevelLoader loader;
    LevelData data = loader.loadLevel(levelName);

    if (resourceManager) {
        loader.populateGameState(data, *this, *resourceManager);
    }

    currentLevel = levelName;
    std::cout << "Level Loaded: " << levelName << " (" << data.width << "x" << data.height << ")" << std::endl;
}

void GameState::transitionToLevel(const std::string& levelName) {
    nextLevel = levelName;
    levelTransitionPending = true;
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