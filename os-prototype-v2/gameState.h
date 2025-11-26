#pragma once
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <SDL3/SDL.h>
#include "Entity.h"
#include "Player.h"
#include "ResourceManager.h"
#include "UIManager.h"

// Layer indices
const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_FURNITURE_BACKGROUND = 1;
const size_t LAYER_IDX_PORTAL_BACKGROUND = 2;
const size_t LAYER_IDX_CHARACTERS = 3;
const size_t LAYER_IDX_PROJECTILES = 4;
const size_t LAYER_IDX_PORTAL_FOREGROUND = 5;
const size_t LAYER_IDX_FURNITURE_FOREGROUND = 6;
const size_t TOTAL_LAYERS = 7;

enum class GameStateMode {
    PLAYING,
    PAUSED,
    CHATTING
};

class GameState {
private:
    GameStateMode mode;
    std::array<std::vector<std::unique_ptr<Entity>>, TOTAL_LAYERS> layers;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<UIManager> uiManager;

    SDL_FRect mapViewport;
    Player* playerPtr;

    int logicalWidth;
    int logicalHeight;

    std::string currentLevel;
    std::string nextLevel;
    bool levelTransitionPending;

    float collisionCheckRadius;

public:
    GameState(SDL_Renderer* renderer, int viewportWidth, int viewportHeight);

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);

    // Entity management
    void addEntity(std::unique_ptr<Entity> entity, size_t layer);
    void moveEntityToLayer(Entity* entity, size_t newLayer);
    void cleanup();

    // Collision detection
    void checkCollisions();
    void checkPlayerCollisions();
    void checkProjectileCollisions();
    void checkMeleeAttacks();
    void checkEnemyAttacks(); // NEW: Enemy damage to player

    // Spatial queries
    std::vector<Entity*> getEntitiesInLayer(size_t layer);
    std::vector<Entity*> getEntitiesInRange(const glm::vec2& pos, float range, size_t layer);
    std::vector<Entity*> getEnemiesInRange(const glm::vec2& pos, float range);

    // Level management
    void loadLevel(const std::string& levelName = "abyss");
    void transitionToLevel(const std::string& levelName);
    void clearLevel();

    // Camera
    void updateCamera();

    // Getters
    Player* getPlayer() { return playerPtr; }
    ResourceManager& getResourceManager() { return *resourceManager; }
    UIManager& getUIManager() { return *uiManager; }
    SDL_FRect getViewport() const { return mapViewport; }
    GameStateMode getMode() const { return mode; }
    std::string getCurrentLevel() const { return currentLevel; }

    // Setters
    void setMode(GameStateMode newMode) { mode = newMode; }
};