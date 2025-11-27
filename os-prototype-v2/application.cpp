#include "Application.h"
#include "FriendlyNpc.h"
#include "GameState.h"
#include <iostream>

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
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_E) {
            if (!FriendlyNPC::activeChatNPC) {
                // Only open chat if not already chatting
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
            // If already chatting, the E key was already handled above and won't reach here
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