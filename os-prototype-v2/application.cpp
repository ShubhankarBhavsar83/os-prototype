#include "Application.h"
#include "FriendlyNpc.h"
#include "GameState.h"
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
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), nullptr);
        return false;
    }

    if (!TTF_Init()) {
        SDL_Log("TTF Error: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("OS-PROTOTYPE", screenWidth, screenHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        return false;
    }

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    keyboardState = SDL_GetKeyboardState(nullptr);
    gameState = std::make_unique<GameState>(renderer, logicalWidth, logicalHeight);
    gameState->loadLevel();

    // Update screen dimensions for all NPCs after loading
    updateNPCScreenDimensions();

    running = true;
    previousTime = SDL_GetTicks();
    std::cout << "Application initialized successfully." << std::endl;
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
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void Application::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        // 1. HANDLE ACTIVE CHAT (Priority handling)
        if (FriendlyNPC::activeChatNPC) {
            if (event.type == SDL_EVENT_TEXT_INPUT) {
                std::cout << "[App] Text input received: " << event.text.text << std::endl;
                FriendlyNPC::activeChatNPC->handleTextInput(event.text.text);
                continue;
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                std::cout << "[App] Key down in chat: " << SDL_GetKeyName(event.key.key) << std::endl;

                // Only ESCAPE closes the chat
                if (event.key.key == SDLK_ESCAPE) {
                    std::cout << "[App] Closing chat with ESC" << std::endl;
                    FriendlyNPC::activeChatNPC->endChat();
                    continue;
                }

                // Handle special keys (backspace, return)
                if (event.key.key == SDLK_BACKSPACE || event.key.key == SDLK_RETURN) {
                    FriendlyNPC::activeChatNPC->handleKeyDown(event.key.key);
                    continue;
                }

                // Convert letter keys to text
                if (event.key.key >= SDLK_A && event.key.key <= SDLK_Z) {
                    char c = (char)event.key.key;
                    std::string text(1, c);
                    FriendlyNPC::activeChatNPC->handleTextInput(text);
                    continue;
                }

                // Handle number keys (0-9)
                if (event.key.key >= SDLK_0 && event.key.key <= SDLK_9) {
                    char c = (char)event.key.key;
                    std::string text(1, c);
                    FriendlyNPC::activeChatNPC->handleTextInput(text);
                    continue;
                }

                // Handle space
                if (event.key.key == SDLK_SPACE) {
                    FriendlyNPC::activeChatNPC->handleTextInput(" ");
                    continue;
                }

                // Handle common punctuation and symbols
                switch (event.key.key) {
                case SDLK_PERIOD:       FriendlyNPC::activeChatNPC->handleTextInput("."); break;
                case SDLK_COMMA:        FriendlyNPC::activeChatNPC->handleTextInput(","); break;
                case SDLK_APOSTROPHE:   FriendlyNPC::activeChatNPC->handleTextInput("'"); break;
                case SDLK_SEMICOLON:    FriendlyNPC::activeChatNPC->handleTextInput(";"); break;
                case SDLK_SLASH:        FriendlyNPC::activeChatNPC->handleTextInput("/"); break;
                case SDLK_MINUS:        FriendlyNPC::activeChatNPC->handleTextInput("-"); break;
                case SDLK_EQUALS:       FriendlyNPC::activeChatNPC->handleTextInput("="); break;
                case SDLK_LEFTBRACKET:  FriendlyNPC::activeChatNPC->handleTextInput("["); break;
                case SDLK_RIGHTBRACKET: FriendlyNPC::activeChatNPC->handleTextInput("]"); break;
                case SDLK_BACKSLASH:    FriendlyNPC::activeChatNPC->handleTextInput("\\"); break;
                case SDLK_GRAVE:        FriendlyNPC::activeChatNPC->handleTextInput("`"); break;
                }

                continue;
            }
        }

        // 2. HANDLE GAME INTERACTIONS (When NOT chatting)
        if (event.type == SDL_EVENT_KEY_DOWN) {
            // E key - NPC interaction
            if (event.key.key == SDLK_E && !FriendlyNPC::activeChatNPC) {
                Player* p = gameState->getPlayer();
                if (p) {
                    FriendlyNPC* npc = gameState->findNearestFriendlyNPC(p->getPosition(), 70.0f);
                    if (npc) {
                        std::cout << "[App] E pressed - Starting chat with NPC" << std::endl;
                        npc->onPlayerInteract(p);
                    }
                    else {
                        std::cout << "[App] No NPC nearby to talk to." << std::endl;
                    }
                }
            }

            // TAB key - Target cycling (NEW from old version)
            if (event.key.key == SDLK_TAB && !FriendlyNPC::activeChatNPC) {
                Player* p = gameState->getPlayer();
                if (p) {
                    p->cycleTarget(*gameState);
                    std::cout << "[App] Target cycled" << std::endl;
                }
            }

            // ESC key - Pause game
            if (event.key.key == SDLK_ESCAPE && !FriendlyNPC::activeChatNPC) {
                gameState->setMode(GameStateMode::PAUSED);
                std::cout << "[App] Game Paused" << std::endl;
            }
        }

        // 3. HANDLE WINDOW EVENTS
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
    // Only move player if NOT chatting
    if (gameState->getMode() == GameStateMode::PLAYING && !FriendlyNPC::activeChatNPC) {
        Player* player = gameState->getPlayer();
        if (player) player->handleInput(keyboardState, *gameState);
    }
    gameState->update(deltaTime);
}

void Application::render() {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    gameState->render(renderer);

    // ========================================================================
    // DEBUG INFO - Left side (RESTORED from old version)
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
    // DEBUG INFO - Top Right (Enemy Health Display) (RESTORED from old version)
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

    // Update screen dimensions for all NPCs when window is resized
    updateNPCScreenDimensions();
}

void Application::updateNPCScreenDimensions() {
    if (!gameState) return;

    // Get all entities in the characters layer and update FriendlyNPCs
    std::vector<Entity*> entities = gameState->getEntitiesInRange(
        glm::vec2(0, 0),
        100000.0f,  // Large range to get all entities
        LAYER_IDX_CHARACTERS
    );

    for (Entity* entity : entities) {
        if (entity->getType() == EntityType::FRIENDLY_NPC) {
            FriendlyNPC* npc = static_cast<FriendlyNPC*>(entity);
            npc->setScreenDimensions(logicalWidth, logicalHeight);
        }
    }

    std::cout << "[App] Updated " << entities.size() << " NPC screen dimensions to "
        << logicalWidth << "x" << logicalHeight << std::endl;
}