#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include "GameState.h"

class Application {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;

    int screenWidth;
    int screenHeight;
    int logicalWidth;
    int logicalHeight;

    const bool* keyboardState;

    std::unique_ptr<GameState> gameState;

    uint64_t previousTime;

public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();

    void handleKeyPress(SDL_Scancode key);

private:
    void processEvents();
    void update(float deltaTime);
    void render();

    void handleWindowResize(int width, int height);
};