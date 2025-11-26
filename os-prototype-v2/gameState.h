#pragma once
#include <vector>
#include <memory>
#include <array>
#include <SDL3/SDL.h>
#include "Entity.h"
#include "Player.h"
#include "ResourceManager.h"

// Forward declaration to avoid circular includes
class FriendlyNPC;

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_FURNITURE_BACKGROUND = 1;
const size_t LAYER_IDX_PORTAL_BACKGROUND = 2;
const size_t LAYER_IDX_CHARACTERS = 3;
const size_t LAYER_IDX_PROJECTILES = 4;  // <-- ADD THIS
const size_t LAYER_IDX_PORTAL_FOREGROUND = 5;
const size_t LAYER_IDX_FURNITURE_FOREGROUND = 6;
const size_t TOTAL_LAYERS = 7;  // <-- UPDATE THIS

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
    SDL_FRect mapViewport;
    Player* playerPtr;
    int logicalWidth;
    int logicalHeight;

public:
    GameState(SDL_Renderer* renderer, int viewportWidth, int viewportHeight);

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void checkCollisions();
    void cleanup();
    void addEntity(std::unique_ptr<Entity> entity, size_t layer);
    void moveEntityToLayer(Entity* entity, size_t newLayer);

    // Helper functions
    FriendlyNPC* findNearestFriendlyNPC(glm::vec2 position, float range);
    std::vector<Entity*> getEnemiesInRange(glm::vec2 position, float range);
    std::vector<Entity*> getEntitiesInRange(glm::vec2 position, float range, size_t layer);

    // Getters
    Player* getPlayer() { return playerPtr; }
    ResourceManager& getResourceManager() { return *resourceManager; }
    SDL_FRect getViewport() const { return mapViewport; }
    GameStateMode getMode() const { return mode; }
    void setMode(GameStateMode newMode) { mode = newMode; }

    void updateCamera();
    void loadLevel();
    void loadLevel(const std::string& levelName);
};
