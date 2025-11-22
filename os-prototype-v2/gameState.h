#pragma once
#include <vector>
#include <memory>
#include <array>
#include <SDL3/SDL.h>
#include "Entity.h"
#include "Player.h"
#include "ResourceManager.h"

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_FURNITURE_BACKGROUND = 1;
const size_t LAYER_IDX_CHARACTERS = 2;
const size_t LAYER_IDX_FURNITURE_FOREGROUND = 3;

enum class GameStateMode {
    PLAYING,
    PAUSED,
    CHATTING
};

class GameState {
private:
    GameStateMode mode;
    std::array<std::vector<std::unique_ptr<Entity>>, 4> layers;
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

    Player* getPlayer() { return playerPtr; }
    ResourceManager& getResourceManager() { return *resourceManager; }
    SDL_FRect getViewport() const { return mapViewport; }
    GameStateMode getMode() const { return mode; }
    void setMode(GameStateMode newMode) { mode = newMode; }

    void updateCamera();
    void loadTestLevel();
};