#include "Application.h"
#include <iostream>
#include <format>

Application::Application()
    : window(nullptr), renderer(nullptr), running(false),
    screenWidth(1280), screenHeight(720),
    logicalWidth(640), logicalHeight(320),
    keyboardState(nullptr), previousTime(0) {
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            SDL_GetError(), nullptr);
        return false;
    }

    // Create window
    window = SDL_CreateWindow("OS-PROTOTYPE", screenWidth, screenHeight,
        SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            SDL_GetError(), nullptr);
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            SDL_GetError(), window);
        return false;
    }

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Get keyboard state
    keyboardState = SDL_GetKeyboardState(nullptr);

    // Create game state
    gameState = std::make_unique<GameState>(renderer, logicalWidth, logicalHeight);
    gameState->loadTestLevel();

    running = true;
    previousTime = SDL_GetTicks();

    std::cout << "Application initialized successfully" << std::endl;
    return true;
}

void Application::run() {
    while (running) {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - previousTime) / 1000.0f;

        // Cap delta time to prevent huge jumps
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        processEvents();
        update(deltaTime);
        render();

        previousTime = nowTime;
    }
}

void Application::shutdown() {
    gameState.reset();

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();

    std::cout << "Application shutdown complete" << std::endl;
}

void Application::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            running = false;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            handleWindowResize(event.window.data1, event.window.data2);
            break;
        }
    }
}

void Application::update(float deltaTime) {
    if (gameState->getMode() == GameStateMode::PLAYING) {
        Player* player = gameState->getPlayer();
        if (player) {
            player->handleInput(keyboardState, *gameState);
        }
    }
    gameState->update(deltaTime);
}

void Application::render() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    gameState->render(renderer);

    // Debug info
    Player* player = gameState->getPlayer();
    if (player) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        std::string stateStr = std::format("State: {}", static_cast<int>(player->getState()));
        SDL_RenderDebugText(renderer, 5, 5, stateStr.c_str());

        glm::vec2 pos = player->getPosition();
        std::string posStr = std::format("Pos: ({:.1f}, {:.1f})", pos.x, pos.y);
        SDL_RenderDebugText(renderer, 5, 20, posStr.c_str());
    }

    SDL_RenderPresent(renderer);
}

void Application::handleWindowResize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}