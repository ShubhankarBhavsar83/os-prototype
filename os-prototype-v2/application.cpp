#include "Application.h"
#include "FriendlyNpc.h" 
#include <iostream>
#include <format>

Application::Application()
    : window(nullptr), renderer(nullptr), running(false),
    screenWidth(1280), screenHeight(720),
    logicalWidth(640), logicalHeight(320),
    keyboardState(nullptr), previousTime(0) {
}

Application::~Application() { shutdown(); }

bool Application::initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), nullptr);
        return false;
    }
    if (!TTF_Init()) { SDL_Log("TTF Error: %s", SDL_GetError()); return false; }

    window = SDL_CreateWindow("OS-PROTOTYPE", screenWidth, screenHeight, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, nullptr);

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    keyboardState = SDL_GetKeyboardState(nullptr);
    gameState = std::make_unique<GameState>(renderer, logicalWidth, logicalHeight);
    gameState->loadLevel();

    running = true;
    previousTime = SDL_GetTicks();
    std::cout << "Application initialized." << std::endl;
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

                // Special handling for movement keys - convert to text
                if (event.key.key >= SDLK_A && event.key.key <= SDLK_Z) {
                    // Convert keycode to character
                    char c = (char)event.key.key;
                    std::string text(1, c);
                    std::cout << "[App] Converting key to text: '" << text << "'" << std::endl;
                    FriendlyNPC::activeChatNPC->handleTextInput(text);
                }
                // Special keys
                else if (event.key.key == SDLK_SPACE) {
                    FriendlyNPC::activeChatNPC->handleTextInput(" ");
                }
                else if (event.key.key == SDLK_BACKSPACE ||
                    event.key.key == SDLK_RETURN ||
                    event.key.key == SDLK_ESCAPE) {
                    FriendlyNPC::activeChatNPC->handleKeyDown(event.key.key);
                }

                continue; // Block all keys from game
            }
        }

        // 2. HANDLE GAME INTERACTIONS (When NOT chatting)
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_E) {
            Player* p = gameState->getPlayer();
            if (p) {
                // Find NPC within 70 pixels
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
}
