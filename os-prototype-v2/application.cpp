#include "Application.h"
#include "EnemyNpc.h"
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
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            SDL_GetError(), nullptr);
        return false;
    }

    window = SDL_CreateWindow("OS-PROTOTYPE", screenWidth, screenHeight,
        SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            SDL_GetError(), nullptr);
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            SDL_GetError(), window);
        return false;
    }

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);

    keyboardState = SDL_GetKeyboardState(nullptr);

    gameState = std::make_unique<GameState>(renderer, logicalWidth, logicalHeight);
    gameState->loadLevel("abyss");

    running = true;
    previousTime = SDL_GetTicks();

    std::cout << "Application initialized successfully" << std::endl;
    return true;
}

void Application::run() {
    while (running) {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - previousTime) / 1000.0f;

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

        case SDL_EVENT_KEY_DOWN:
            if (event.key.repeat) break;
            handleKeyPress(event.key.scancode);
            break;
        }
    }
}

void Application::handleKeyPress(SDL_Scancode key) {
    if (gameState->getMode() != GameStateMode::PLAYING) return;

    Player* player = gameState->getPlayer();
    if (!player) return;

    switch (key) {
    case SDL_SCANCODE_TAB:
        player->cycleTarget(*gameState);
        std::cout << "Target cycled" << std::endl;
        break;

    case SDL_SCANCODE_E:
        player->interact(*gameState);
        break;

    case SDL_SCANCODE_ESCAPE:
        gameState->setMode(GameStateMode::PAUSED);
        std::cout << "Game Paused" << std::endl;
        break;

    default:
        break;
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

    // ========================================================================
    // DEBUG INFO - Left side
    // ========================================================================
    Player* player = gameState->getPlayer();
    if (player) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        std::string stateStr = std::format("State: {}", static_cast<int>(player->getState()));
        SDL_RenderDebugText(renderer, 5, 5, stateStr.c_str());

        glm::vec2 pos = player->getPosition();
        std::string posStr = std::format("Pos: ({:.1f}, {:.1f})", pos.x, pos.y);
        SDL_RenderDebugText(renderer, 5, 20, posStr.c_str());

        std::string hpStr = std::format("HP: {:.0f}/{:.0f}",
            player->getHealth(), player->getMaxHealth());
        SDL_RenderDebugText(renderer, 5, 35, hpStr.c_str());

        if (player->getTarget()) {
            SDL_RenderDebugText(renderer, 5, 50, "Target: Locked");
        }
    }

    // ========================================================================
    // DEBUG INFO - Top Right (Enemy Health Display)
    // ========================================================================
    if (player && player->getTarget()) {
        Entity* target = player->getTarget();

        if (target->getType() == EntityType::ENEMY) {
            EnemyNPC* enemy = static_cast<EnemyNPC*>(target);

            // Position at top right corner
            int rightAlignX = logicalWidth - 150;
            int startY = 5;

            SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);

            // Enemy type
            std::string variantStr;
            switch (enemy->getVariant()) {
            case EnemyVariant::BEAST_MELEE:
                variantStr = "Beast";
                break;
            case EnemyVariant::HALBERD_FIGHTER:
                variantStr = "Halberd Fighter";
                break;
            case EnemyVariant::BOSS_MELEE:
                variantStr = "BOSS";
                SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
                break;
            }

            SDL_RenderDebugText(renderer, rightAlignX, startY,
                std::format("Target: {}", variantStr).c_str());

            // HP display with color coding
            float hpPercent = enemy->getHealthPercent();
            if (hpPercent > 0.6f) {
                SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); // Red (enemy color)
            }
            else if (hpPercent > 0.3f) {
                SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255); // Orange
            }
            else {
                SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // Light red (low HP)
            }

            std::string enemyHPStr = std::format("HP: {:.0f}/{:.0f}",
                enemy->getHealth(), enemy->getMaxHealth());
            SDL_RenderDebugText(renderer, rightAlignX, startY + 15, enemyHPStr.c_str());

            // HP bar visualization
            int barWidth = 140;
            int barHeight = 8;
            int barX = rightAlignX;
            int barY = startY + 30;

            // Background
            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 200);
            SDL_FRect bgRect = {
                static_cast<float>(barX),
                static_cast<float>(barY),
                static_cast<float>(barWidth),
                static_cast<float>(barHeight)
            };
            SDL_RenderFillRect(renderer, &bgRect);

            // HP Fill
            SDL_FRect fillRect = {
                static_cast<float>(barX),
                static_cast<float>(barY),
                static_cast<float>(barWidth) * hpPercent,
                static_cast<float>(barHeight)
            };

            if (hpPercent > 0.6f) {
                SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            }
            else if (hpPercent > 0.3f) {
                SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
            SDL_RenderFillRect(renderer, &fillRect);

            // Border
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderRect(renderer, &bgRect);
        }
    }

    SDL_RenderPresent(renderer);
}

void Application::handleWindowResize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}